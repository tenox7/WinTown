/* notifications.c - Enhanced notification system implementation for MicropolisNT
 * Based on original Micropolis notification analysis
 */

#include "notifications.h"
#include "sim.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* External functions */
extern HWND hwndMain;
extern void addGameLog(const char *format, ...);

/* Global notification data */
static Notification currentNotification;

/* Notification templates - organized by ID */
typedef struct {
    int id;
    NotificationType type;
    const char* title;
    const char* message;
    const char* explanation;
    const char* advice;
} NotificationTemplate;

/* Emergency Notifications */
static NotificationTemplate emergencyNotifications[] = {
    {
        NOTIF_FIRE_REPORTED,
        NOTIF_EMERGENCY,
        "Fire Emergency",
        "Fire reported in %s!",
        "A fire has broken out and is threatening nearby buildings. Fires can spread rapidly if not contained quickly by fire departments.",
        "Deploy fire trucks immediately. Ensure adequate fire department coverage and funding. Consider building additional fire stations in high-risk areas."
    },
    {
        NOTIF_EARTHQUAKE,
        NOTIF_EMERGENCY,
        "Major Earthquake",
        "Major earthquake reported! Magnitude estimated at %d.%d",
        "A powerful earthquake has struck your city, causing widespread damage to buildings and infrastructure. Emergency services are responding to multiple incidents.",
        "Assess damage citywide. Rebuild destroyed infrastructure. Ensure emergency services are fully funded for rapid response to secondary disasters."
    },
    {
        NOTIF_NUCLEAR_MELTDOWN,
        NOTIF_EMERGENCY,
        "Nuclear Emergency",
        "NUCLEAR MELTDOWN has occurred at power plant!",
        "A catastrophic failure at a nuclear power plant has resulted in a meltdown. Radiation is spreading across the surrounding area, making it uninhabitable.",
        "Evacuate surrounding areas immediately. The contaminated zone will remain uninhabitable for many years. Consider alternative power sources for the future."
    },
    {
        NOTIF_MONSTER_SIGHTED,
        NOTIF_EMERGENCY,
        "Monster Attack",
        "A Monster has been sighted in the city!",
        "A giant monster, possibly awakened by high pollution levels, is attacking the city. It is destroying buildings and infrastructure as it moves.",
        "Military forces are responding. High pollution levels may have attracted this creature. Reduce industrial pollution to prevent future attacks."
    }
};

/* Infrastructure Notifications */
static NotificationTemplate infrastructureNotifications[] = {
    {
        NOTIF_BLACKOUTS,
        NOTIF_WARNING,
        "Power Crisis",
        "Blackouts reported across the city!",
        "Large portions of your city are without electrical power. This occurs when power demand exceeds supply or when the power grid is inadequate.",
        "Build additional power plants to meet demand. Ensure power lines connect all areas. Check the power map to identify problem areas."
    },
    {
        NOTIF_TRAFFIC_JAMS,
        NOTIF_WARNING,
        "Traffic Problems",
        "Frequent traffic jams reported throughout the city.",
        "Heavy traffic congestion is slowing commerce and reducing quality of life. This typically occurs when road capacity cannot handle the traffic volume.",
        "Build more roads to increase capacity. Consider mass transit options like rail systems. Ensure efficient road network design with minimal bottlenecks."
    },
    {
        NOTIF_ROADS_DETERIORATING,
        NOTIF_WARNING,
        "Infrastructure Decay",
        "Roads are deteriorating due to lack of maintenance funding.",
        "Insufficient funding for road maintenance is causing the transportation network to decay. Poor roads reduce traffic efficiency and economic growth.",
        "Increase road funding in the budget window. Maintain at least 70% funding level for optimal road conditions. Consider raising taxes if necessary."
    },
    {
        NOTIF_MORE_ROADS_NEEDED,
        NOTIF_INFO,
        "Transportation Needs",
        "More roads required for growing traffic.",
        "Your city's traffic volume is exceeding the capacity of the current road network. Citizens are experiencing delays and reduced mobility.",
        "Build more roads to connect all areas of your city. Consider building a comprehensive road network before the city grows larger."
    },
    {
        NOTIF_RAIL_SYSTEM_NEEDED,
        NOTIF_INFO,
        "Mass Transit",
        "Rail system needed for efficient transportation.",
        "Your city has grown large enough to benefit from mass transit. Rail systems move large numbers of people efficiently and reduce traffic congestion.",
        "Build rail lines connecting major population and commercial centers. Rails are more efficient than roads for high-capacity transit."
    },
    {
        NOTIF_POWER_PLANT_NEEDED,
        NOTIF_WARNING,
        "Power Shortage",
        "Build a Power Plant to supply electricity.",
        "Your city needs electrical power to function. Commercial and industrial zones require electricity to operate, and residential areas need power for quality of life.",
        "Build a coal or nuclear power plant. Connect all zones to the power grid with power lines. Nuclear plants are more efficient but riskier."
    },
    {
        NOTIF_FIRE_DEPT_UNDERFUNDED,
        NOTIF_WARNING,
        "Service Funding",
        "Fire departments need more funding.",
        "Insufficient funding is reducing fire department effectiveness. Poorly funded fire departments cannot respond quickly to emergencies and may not prevent fire spread.",
        "Increase fire department funding in the budget window. Maintain at least 70% funding for effective fire protection."
    },
    {
        NOTIF_POLICE_UNDERFUNDED,
        NOTIF_WARNING,
        "Service Funding", 
        "Police departments need more funding.",
        "Insufficient funding is reducing police effectiveness. Poorly funded police departments cannot adequately patrol the city and prevent crime.",
        "Increase police department funding in the budget window. Maintain at least 70% funding for effective crime prevention."
    }
};

/* Zoning Notifications */
static NotificationTemplate zoningNotifications[] = {
    {
        NOTIF_RESIDENTIAL_NEEDED,
        NOTIF_INFO,
        "Housing Shortage",
        "More residential zones needed for growing population.",
        "Your city's population is growing faster than available housing. Workers need places to live near their jobs for optimal city development.",
        "Zone more residential areas near commercial and industrial zones. Ensure adequate transportation links between residential and job centers."
    },
    {
        NOTIF_COMMERCIAL_NEEDED,
        NOTIF_INFO,
        "Commercial Development",
        "More commercial zones needed to serve the population.",
        "Your growing population needs more shops, offices, and services. Commercial zones provide jobs and serve the daily needs of residents.",
        "Zone commercial areas near residential populations. Ensure good transportation access and adequate power supply for commercial development."
    },
    {
        NOTIF_INDUSTRIAL_NEEDED,
        NOTIF_INFO,
        "Industrial Development",
        "More industrial zones needed for economic growth.",
        "Your city needs more industrial development to provide jobs and produce goods. Industrial zones are the foundation of a strong economy.",
        "Zone industrial areas away from residential zones to avoid pollution complaints. Ensure good transportation access and power supply."
    },
    {
        NOTIF_STADIUM_NEEDED,
        NOTIF_INFO,
        "Entertainment Demand",
        "Residents demand a Stadium for entertainment.",
        "Your population has reached a size where citizens expect recreational facilities. A stadium will improve quality of life and attract more residents.",
        "Build a stadium in an accessible location with good transportation links. This will boost residential development and citizen satisfaction."
    },
    {
        NOTIF_SEAPORT_NEEDED,
        NOTIF_INFO,
        "Industrial Infrastructure",
        "Industry requires a Seaport for trade.",
        "Your industrial sector has grown large enough to need import/export facilities. A seaport will boost industrial growth and economic development.",
        "Build a seaport connected to your industrial zones. Ensure good transportation links between the port and industrial areas."
    },
    {
        NOTIF_AIRPORT_NEEDED,
        NOTIF_INFO,
        "Commercial Infrastructure", 
        "Commerce requires an Airport for growth.",
        "Your commercial sector needs air transport for business travel and high-value goods. An airport will attract businesses and boost commercial development.",
        "Build an airport with good connections to commercial zones. Airports require large amounts of space and should be placed away from residential areas."
    },
    {
        NOTIF_FIRE_DEPT_NEEDED,
        NOTIF_WARNING,
        "Public Safety",
        "Citizens demand a Fire Department.",
        "Your city has grown large enough to need professional fire protection. Fire departments respond to emergencies and prevent fire spread.",
        "Build fire stations throughout your city for adequate coverage. Each station protects a limited area, so multiple stations may be needed."
    },
    {
        NOTIF_POLICE_NEEDED,
        NOTIF_WARNING,
        "Public Safety",
        "Citizens demand a Police Department.",
        "Your city needs law enforcement to maintain order and prevent crime. Police departments patrol the city and reduce criminal activity.",
        "Build police stations for crime prevention. Each station covers a limited area, so larger cities need multiple stations for full coverage."
    }
};

/* Financial Notifications */
static NotificationTemplate financialNotifications[] = {
    {
        NOTIF_CITY_BROKE,
        NOTIF_FINANCIAL,
        "Financial Crisis",
        "YOUR CITY HAS GONE BROKE!",
        "The city treasury is completely depleted and automatic budget allocation has failed. Essential services may be suspended without immediate action.",
        "Review and adjust the budget immediately. Consider raising tax rates or reducing service funding. Focus spending on essential services only."
    },
    {
        NOTIF_TAX_TOO_HIGH,
        NOTIF_WARNING,
        "Tax Burden",
        "Citizens are upset - the tax rate is too high!",
        "High taxes are causing citizen dissatisfaction and may lead to population decline. People and businesses may leave for cities with lower tax rates.",
        "Consider reducing tax rates to improve citizen satisfaction. Look for ways to reduce spending or improve efficiency instead of raising taxes."
    }
};

/* Milestone Notifications */
static NotificationTemplate milestoneNotifications[] = {
    {
        NOTIF_VILLAGE_2K,
        NOTIF_MILESTONE,
        "Village Achievement",
        "Congratulations! Your settlement has grown to 2,000 people!",
        "Your small settlement has officially become a village. This represents the first major milestone in your city's development journey.",
        "Continue balanced development of residential, commercial, and industrial zones. Plan for future growth with adequate infrastructure."
    },
    {
        NOTIF_TOWN_10K,
        NOTIF_MILESTONE,
        "Town Status",
        "Your village has grown into a town of 10,000 citizens!",
        "This significant population milestone marks your transition from a village to a proper town with all the opportunities and challenges that brings.",
        "Focus on infrastructure development and public services. Consider specialized buildings like fire and police stations."
    },
    {
        NOTIF_CITY_50K,
        NOTIF_MILESTONE,
        "City Status",
        "Your town has become a city with 50,000 residents!",
        "Achieving city status is a major accomplishment. Your growing metropolis now faces more complex challenges in management and planning.",
        "Invest in advanced infrastructure, public transportation, and specialized services. Monitor crime, pollution, and traffic carefully."
    }
};

/* Initialize notification system */
void InitNotificationSystem(void) {
    /* Initialize any global notification system state */
    memset(&currentNotification, 0, sizeof(Notification));
}

/* Find notification template by ID */
static NotificationTemplate* FindNotificationTemplate(int notificationId) {
    int i;
    
    /* Search emergency notifications */
    for (i = 0; i < sizeof(emergencyNotifications) / sizeof(NotificationTemplate); i++) {
        if (emergencyNotifications[i].id == notificationId) {
            return &emergencyNotifications[i];
        }
    }
    
    /* Search infrastructure notifications */
    for (i = 0; i < sizeof(infrastructureNotifications) / sizeof(NotificationTemplate); i++) {
        if (infrastructureNotifications[i].id == notificationId) {
            return &infrastructureNotifications[i];
        }
    }
    
    /* Search zoning notifications */
    for (i = 0; i < sizeof(zoningNotifications) / sizeof(NotificationTemplate); i++) {
        if (zoningNotifications[i].id == notificationId) {
            return &zoningNotifications[i];
        }
    }
    
    /* Search financial notifications */
    for (i = 0; i < sizeof(financialNotifications) / sizeof(NotificationTemplate); i++) {
        if (financialNotifications[i].id == notificationId) {
            return &financialNotifications[i];
        }
    }
    
    /* Search milestone notifications */
    for (i = 0; i < sizeof(milestoneNotifications) / sizeof(NotificationTemplate); i++) {
        if (milestoneNotifications[i].id == notificationId) {
            return &milestoneNotifications[i];
        }
    }
    
    return NULL;
}

/* Show notification dialog */
void ShowNotification(int notificationId, ...) {
    NotificationTemplate* template;
    va_list args;
    
    template = FindNotificationTemplate(notificationId);
    if (!template) {
        return;
    }
    
    /* Prepare notification structure */
    currentNotification.id = notificationId;
    currentNotification.type = template->type;
    currentNotification.hasLocation = 0;
    currentNotification.priority = (template->type == NOTIF_EMERGENCY) ? 3 : 1;
    currentNotification.timestamp = GetTickCount();
    
    /* Copy title */
    strncpy(currentNotification.title, template->title, sizeof(currentNotification.title) - 1);
    currentNotification.title[sizeof(currentNotification.title) - 1] = '\0';
    
    /* Format message with arguments */
    va_start(args, notificationId);
    vsprintf(currentNotification.message, template->message, args);
    va_end(args);
    
    /* Copy explanation and advice */
    strncpy(currentNotification.explanation, template->explanation, sizeof(currentNotification.explanation) - 1);
    currentNotification.explanation[sizeof(currentNotification.explanation) - 1] = '\0';
    
    strncpy(currentNotification.advice, template->advice, sizeof(currentNotification.advice) - 1);
    currentNotification.advice[sizeof(currentNotification.advice) - 1] = '\0';
    
    /* Also log to game log */
    addGameLog("%s: %s", currentNotification.title, currentNotification.message);
    
    /* Show dialog */
    CreateNotificationDialog(&currentNotification);
}

/* Show notification with location */
void ShowNotificationAt(int notificationId, int x, int y, ...) {
    NotificationTemplate* template;
    va_list args;
    
    template = FindNotificationTemplate(notificationId);
    if (!template) {
        return;
    }
    
    /* Prepare notification structure */
    currentNotification.id = notificationId;
    currentNotification.type = template->type;
    currentNotification.hasLocation = 1;
    currentNotification.locationX = x;
    currentNotification.locationY = y;
    currentNotification.priority = (template->type == NOTIF_EMERGENCY) ? 3 : 1;
    currentNotification.timestamp = GetTickCount();
    
    /* Copy title */
    strncpy(currentNotification.title, template->title, sizeof(currentNotification.title) - 1);
    currentNotification.title[sizeof(currentNotification.title) - 1] = '\0';
    
    /* Format message with arguments */
    va_start(args, y);
    vsprintf(currentNotification.message, template->message, args);
    va_end(args);
    
    /* Copy explanation and advice */
    strncpy(currentNotification.explanation, template->explanation, sizeof(currentNotification.explanation) - 1);
    currentNotification.explanation[sizeof(currentNotification.explanation) - 1] = '\0';
    
    strncpy(currentNotification.advice, template->advice, sizeof(currentNotification.advice) - 1);
    currentNotification.advice[sizeof(currentNotification.advice) - 1] = '\0';
    
    /* Format location */
    sprintf(currentNotification.locationName, "Coordinates: (%d, %d)", x, y);
    
    /* Also log to game log */
    addGameLog("%s at (%d,%d): %s", currentNotification.title, x, y, currentNotification.message);
    
    /* Show dialog */
    CreateNotificationDialog(&currentNotification);
}

/* Create and show notification dialog */
void CreateNotificationDialog(Notification* notif) {
    if (!notif || !hwndMain) {
        return;
    }
    
    /* Show modal dialog */
    DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_NOTIFICATION_DIALOG), 
                   hwndMain, NotificationDialogProc, (LPARAM)notif);
}

/* Dialog procedure for notification dialog */
BOOL CALLBACK NotificationDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static Notification* notif = NULL;
    
    switch (msg) {
        case WM_INITDIALOG:
            notif = (Notification*)lParam;
            if (notif) {
                /* Set dialog title */
                SetWindowText(hwnd, notif->title);
                
                /* Set message text */
                SetDlgItemText(hwnd, IDC_NOTIF_TITLE, notif->title);
                SetDlgItemText(hwnd, IDC_NOTIF_MESSAGE, notif->message);
                SetDlgItemText(hwnd, IDC_NOTIF_EXPLANATION, notif->explanation);
                SetDlgItemText(hwnd, IDC_NOTIF_ADVICE, notif->advice);
                
                /* Set location info */
                if (notif->hasLocation) {
                    SetDlgItemText(hwnd, IDC_NOTIF_LOCATION, notif->locationName);
                    EnableWindow(GetDlgItem(hwnd, IDC_GOTO_LOCATION), TRUE);
                } else {
                    SetDlgItemText(hwnd, IDC_NOTIF_LOCATION, "Location: Not specified");
                    EnableWindow(GetDlgItem(hwnd, IDC_GOTO_LOCATION), FALSE);
                }
            }
            return TRUE;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    return TRUE;
                    
                case IDC_GOTO_LOCATION:
                    if (notif && notif->hasLocation) {
                        CenterMapOnLocation(notif->locationX, notif->locationY);
                        EndDialog(hwnd, IDOK);
                    }
                    return TRUE;
            }
            break;
    }
    
    return FALSE;
}

/* Center map on location - placeholder implementation */
void CenterMapOnLocation(int x, int y) {
    /* This would center the main map view on the specified coordinates */
    /* For now, just log the action */
    addGameLog("Centering map on location (%d, %d)", x, y);
}

/* Enhanced addGameLog that can show dialogs for important notifications */
void addGameLogWithDialog(int showDialog, const char *format, ...) {
    va_list args;
    char buffer[512];
    
    /* Format message */
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    /* Always log to file */
    addGameLog("%s", buffer);
    
    /* Optionally show dialog for important messages */
    if (showDialog) {
        MessageBox(hwndMain, buffer, "City Notification", MB_OK | MB_ICONINFORMATION);
    }
}