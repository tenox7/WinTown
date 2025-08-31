/* budget.c - Budget management system for WiNTown
 * Based on original WiNTown code from WiNTownLegacy project
 */

#include "sim.h"
#include "notify.h"

/* External log functions */
extern void addGameLog(const char *format, ...);
extern void addDebugLog(const char *format, ...);

/* Budget values */
float RoadPercent = 1.0f;   /* Road funding percentage (0.0-1.0) */
float PolicePercent = 1.0f; /* Police funding percentage (0.0-1.0) */
float FirePercent = 1.0f;   /* Fire funding percentage (0.0-1.0) */

QUAD RoadFund = 0;   /* Required road funding amount */
QUAD PoliceFund = 0; /* Required police funding amount */
QUAD FireFund = 0;   /* Required fire funding amount */

QUAD RoadSpend = 0;   /* Actual road spending */
QUAD PoliceSpend = 0; /* Actual police spending */
QUAD FireSpend = 0;   /* Actual fire spending */

QUAD TaxFund = 0;   /* Tax income for current year */
int AutoBudget = 1; /* Auto-budget enabled flag */

/* Budget initialization */
void InitBudget(void) {
    /* Set initial percentages to 100% */
    FirePercent = 1.0f;
    PolicePercent = 1.0f;
    RoadPercent = 1.0f;

    /* Default to auto-budget */
    AutoBudget = 1;

    /* Reset all spending values */
    RoadFund = 0;
    PoliceFund = 0;
    FireFund = 0;
    RoadSpend = 0;
    PoliceSpend = 0;
    FireSpend = 0;
    TaxFund = 0;

    /* Log budget initialization */
    addDebugLog("Budget system initialized: Tax rate %d%%", TaxRate);
    addDebugLog("Starting funds: $%d", (int)TotalFunds);
}

/* Tax collection function - called yearly */
void CollectTax(void) {
    /* No income initially */
    TaxFund = 0;

    /* Calculate funding requirements with difficulty multipliers */
    RoadFund = (QUAD)((RoadTotal + RailTotal) * 1 * DifficultyMaintenanceCost[GameLevel]);     /* Apply maintenance cost multiplier */
    FireFund = FirePop * 100;     /* $100 per fire station */
    PoliceFund = PolicePop * 100; /* $100 per police station */

    /* Log funding requirements */
    addDebugLog("Annual budget requirements:");
    addDebugLog("Roads: $%d (%d tiles, %.1fx difficulty)", (int)RoadFund, RoadTotal + RailTotal, DifficultyMaintenanceCost[GameLevel]);
    addDebugLog("Fire: $%d (%d stations)", (int)FireFund, FirePop);
    addDebugLog("Police: $%d (%d stations)", (int)PoliceFund, PolicePop);

    /* Only process budget if there are people to tax */
    if (TotalPop > 0) {
        /* Calculate tax income using original formula with difficulty multiplier */
        TaxFund = (QUAD)(((TotalPop * LVAverage) / 120) * TaxRate * DifficultyTaxEfficiency[GameLevel]);

        /* Log tax collection */
        addGameLog("Annual tax collection: $%d", (int)TaxFund);
        addDebugLog("Tax details: Rate %d%%, Total pop %d, LV avg %d, Difficulty modifier %.1f", 
                    TaxRate, TotalPop, LVAverage, DifficultyTaxEfficiency[GameLevel]);

        /* Add funds to treasury */
        Spend(-TaxFund);

        /* Update budget to allocate available funds */
        DoBudget();
    } else {
        /* No population - set default service effects without budget processing */
        RoadEffect = 32;
        PoliceEffect = 1000;
        FireEffect = 1000;
        
        /* Reset spending to zero */
        RoadSpend = 0;
        PoliceSpend = 0;
        FireSpend = 0;
        
        addDebugLog("No population - skipping budget processing, using default effects");
    }
}

/* Spend money (negative means income) */
void Spend(QUAD amount) {
    QUAD oldFunds = TotalFunds;

    /* Add to treasury - negative values increase funds */
    TotalFunds -= amount;

    /* Ensure funds never go below zero */
    if (TotalFunds < 0) {
        /* Show enhanced notification dialog */
        ShowNotification(NOTIF_CITY_BROKE);
        
        /* Log funds depleted */
        addGameLog("FINANCIAL CRISIS: City treasury is empty!");
        addDebugLog("Funds depleted: Attempted to spend $%d with only $%d available", (int)amount,
                    (int)oldFunds);
        TotalFunds = 0;
    }

    /* Log major spending/income */
    if (amount > 10000 || amount < -10000) {
        if (amount > 0) {
            addDebugLog("Major expense: $%d (Funds: $%d)", (int)amount, (int)TotalFunds);
        } else {
            addDebugLog("Major income: $%d (Funds: $%d)", (int)-amount, (int)TotalFunds);
        }
    }
}

/* Auto-budget processing */
void DoBudget(void) {
    QUAD total;
    QUAD yumDuckets;
    QUAD fireInt;
    QUAD policeInt;
    QUAD roadInt;
    int fromMenu = 0;  /* Called from budget cycle, not menu */

    /* Calculate desired allocation based on percentages */
    fireInt = (QUAD)(((float)FireFund) * FirePercent);
    policeInt = (QUAD)(((float)PoliceFund) * PolicePercent);
    roadInt = (QUAD)(((float)RoadFund) * RoadPercent);

    total = fireInt + policeInt + roadInt;
    yumDuckets = TaxFund + TotalFunds;

    /* Check if budget window should be shown */
    if (!AutoBudget || (yumDuckets < total && !fromMenu)) {
        extern HWND hwndMain;
        int result;
        
        /* If insufficient funds during auto-budget, disable auto-budget */
        if (AutoBudget && yumDuckets < total) {
            AutoBudget = 0;
            /* Show enhanced notification dialog */
            ShowNotification(NOTIF_BUDGET_DEFICIT);
            addGameLog("BUDGET CRISIS: Insufficient funds - Auto-budget disabled");
        }
        
        /* Show budget window and wait for user input */
        result = ShowBudgetWindowAndWait(hwndMain);
        
        /* If user cancelled, re-enable auto-budget as fallback */
        if (result == IDCANCEL) {
            AutoBudget = 1;
            addGameLog("Budget cancelled - Auto-budget re-enabled");
        }
        
        /* Recalculate with new values after user input */
        fireInt = (QUAD)(((float)FireFund) * FirePercent);
        policeInt = (QUAD)(((float)PoliceFund) * PolicePercent);
        roadInt = (QUAD)(((float)RoadFund) * RoadPercent);
        total = fireInt + policeInt + roadInt;
        yumDuckets = TaxFund + TotalFunds;
    }

    /* If we have enough money for full funding */
    if (yumDuckets >= total) {
        FireSpend = fireInt;
        PoliceSpend = policeInt;
        RoadSpend = roadInt;
    }
    /* If we don't have enough money, allocate in priority order */
    else if (total > 0) {
        /* Try to fund roads first */
        if (yumDuckets > roadInt) {
            RoadSpend = roadInt;
            yumDuckets -= roadInt;

            /* Then try to fund fire protection */
            if (yumDuckets > fireInt) {
                FireSpend = fireInt;
                yumDuckets -= fireInt;

                /* Finally fund police if money remains */
                if (yumDuckets > policeInt) {
                    PoliceSpend = policeInt;
                    yumDuckets -= policeInt;
                } else {
                    /* Partial police funding */
                    PoliceSpend = yumDuckets;
                    if (yumDuckets > 0 && PoliceFund > 0) {
                        PolicePercent = ((float)yumDuckets) / ((float)PoliceFund);
                    } else {
                        PolicePercent = 0.0f;
                    }
                }
            } else {
                /* Partial fire funding */
                FireSpend = yumDuckets;
                PoliceSpend = 0;
                PolicePercent = 0.0f;
                if (yumDuckets > 0 && FireFund > 0) {
                    FirePercent = ((float)yumDuckets) / ((float)FireFund);
                } else {
                    FirePercent = 0.0f;
                }
            }
        } else {
            /* Partial road funding */
            RoadSpend = yumDuckets;
            if (yumDuckets > 0 && RoadFund > 0) {
                RoadPercent = ((float)yumDuckets) / ((float)RoadFund);
            } else {
                RoadPercent = 0.0f;
            }

            FireSpend = 0;
            PoliceSpend = 0;
            FirePercent = 0.0f;
            PolicePercent = 0.0f;
        }
    } else {
        /* No required funding */
        FireSpend = 0;
        PoliceSpend = 0;
        RoadSpend = 0;
        FirePercent = 1.0f;
        PolicePercent = 1.0f;
        RoadPercent = 1.0f;
    }

    /* Calculate effective rates */
    fireInt = FireFund > 0 ? FireSpend * 100 / FireFund : 100;
    policeInt = PoliceFund > 0 ? PoliceSpend * 100 / PoliceFund : 100;
    roadInt = RoadFund > 0 ? RoadSpend * 100 / RoadFund : 100;

    /* Apply effect based on funding level */
    if (FireEffect != (int)fireInt) {
        FireEffect = (int)fireInt;
        /* Effect on fire coverage */
    }

    if (PoliceEffect != (int)policeInt) {
        PoliceEffect = (int)policeInt;
        /* Effect on police coverage */
    }

    if (RoadEffect != (int)roadInt) {
        RoadEffect = (int)roadInt;
        /* Effect on traffic and roads */
    }

    /* Spend budget money */
    total = FireSpend + PoliceSpend + RoadSpend;

    /* Log actual spending */
    addGameLog("Annual budget: Income $%d, Expenses $%d", (int)TaxFund, (int)total);
    addDebugLog("Spending breakdown:");
    addDebugLog("Roads: $%d (%d%% funded)", (int)RoadSpend, RoadEffect);
    addDebugLog("Fire: $%d (%d%% funded)", (int)FireSpend, FireEffect);
    addDebugLog("Police: $%d (%d%% funded)", (int)PoliceSpend, PoliceEffect);
    addDebugLog("Current funds: $%d", (int)TotalFunds);

    /* If funding is low, give a warning */
    if (RoadEffect < 70) {
        addGameLog("WARNING: Road maintenance underfunded (%d%%)", RoadEffect);
    }
    if (FireEffect < 70) {
        addGameLog("WARNING: Fire department underfunded (%d%%)", FireEffect);
    }
    if (PoliceEffect < 70) {
        addGameLog("WARNING: Police department underfunded (%d%%)", PoliceEffect);
    }

    Spend(total);
}

/* Gets current tax income */
QUAD GetTaxIncome(void) {
    return TaxFund;
}

/* Gets current budget balance (after spending) */
QUAD GetBudgetBalance(void) {
    QUAD total = FireSpend + PoliceSpend + RoadSpend;
    return TaxFund - total;
}

/* Returns the effective road maintenance percentage */
int GetRoadEffect(void) {
    return RoadEffect;
}

/* Returns the effective police funding percentage */
int GetPoliceEffect(void) {
    return PoliceEffect;
}

/* Returns the effective fire department funding percentage */
int GetFireEffect(void) {
    return FireEffect;
}

/* Unified budget percentage setter */
void SetBudgetPercent(int budgetType, float percent) {
    /* Validate percentage range */
    if (percent < 0.0f) {
        percent = 0.0f;
    } else if (percent > 1.0f) {
        percent = 1.0f;
    }

    /* Set the appropriate budget percentage */
    switch (budgetType) {
        case BUDGET_TYPE_ROAD:
            RoadPercent = percent;
            break;
        case BUDGET_TYPE_POLICE:
            PolicePercent = percent;
            break;
        case BUDGET_TYPE_FIRE:
            FirePercent = percent;
            break;
        default:
            /* Invalid budget type - do nothing */
            return;
    }

    /* Update budget calculations */
    DoBudget();
}

/* Sets road funding percentage */
void SetRoadPercent(float percent) {
    SetBudgetPercent(BUDGET_TYPE_ROAD, percent);
}

/* Sets police funding percentage */
void SetPolicePercent(float percent) {
    SetBudgetPercent(BUDGET_TYPE_POLICE, percent);
}

/* Sets fire department funding percentage */
void SetFirePercent(float percent) {
    SetBudgetPercent(BUDGET_TYPE_FIRE, percent);
}
