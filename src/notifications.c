/* notifications.c - Simple single-message notification system for MicropolisNT
 * Based on original Micropolis SendMes() function
 */

#include "notifications.h"
#include "sim.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* External functions */
extern HWND hwndMain;
extern void addGameLog(const char *format, ...);
extern void addDebugLog(const char *format, ...);

/* Single message system like original Micropolis */
static int activeNotificationId = 0;  /* 0 = no active notification */
static DWORD notificationStartTime = 0;
static DWORD lastDisasterTime = 0;
static int lastPicNum = 0;            /* Last disaster type shown (like original LastPicNum) */
static int disasterCooldown = 20000;  /* 20 seconds */
static int messageDuration = 30000;   /* 30 seconds */

/* Simple notification check - like original SendMes */
static int CanSendNotification(int notificationId) {
    DWORD currentTime = GetTickCount();
    
    /* Check if current message has timed out */
    if (activeNotificationId != 0 && 
        (currentTime - notificationStartTime) > messageDuration) {
        activeNotificationId = 0;
        addDebugLog("Notification %d timed out, cleared", activeNotificationId);
    }
    
    /* Emergency messages (like original picture messages) - implement LastPicNum logic */
    if (notificationId >= 1000 && notificationId < 2000) {  /* Emergency range */
        /* CRITICAL: Like original - only show if different disaster type than last */
        if (notificationId != lastPicNum) {
            lastDisasterTime = currentTime;
            lastPicNum = notificationId;
            addDebugLog("Emergency notification %d allowed (different from last %d)", notificationId, lastPicNum);
            return 1;
        } else {
            addDebugLog("Emergency notification %d BLOCKED - same as last disaster type", notificationId);
            return 0;  /* Block duplicate disaster types */
        }
    }
    
    /* Block advisory messages during disaster cooldown */
    if (notificationId >= 2000 && 
        (currentTime - lastDisasterTime) < disasterCooldown) {
        addDebugLog("Advisory notification %d blocked by disaster cooldown", notificationId);
        return 0;
    }
    
    /* Block if there's already an active notification */
    if (activeNotificationId != 0) {
        addDebugLog("Notification %d blocked - active: %d", notificationId, activeNotificationId);
        return 0;
    }
    
    return 1;
}

/* Initialize notification system */
void InitNotificationSystem(void) {
    activeNotificationId = 0;
    notificationStartTime = 0;
    lastDisasterTime = 0;
}

/* Simple notification - just show MessageBox for now */
void ShowNotification(int notificationId, ...) {
    char message[256];
    va_list args;
    
    if (!CanSendNotification(notificationId)) {
        return;
    }
    
    /* Set as active */
    activeNotificationId = notificationId;
    notificationStartTime = GetTickCount();
    
    /* Format message */
    va_start(args, notificationId);
    switch (notificationId) {
        case NOTIF_EARTHQUAKE:
            strcpy(message, "Major Earthquake!");
            break;
        case NOTIF_FIRE_REPORTED:
            strcpy(message, "Fire Reported!");
            break;
        case NOTIF_MONSTER_SIGHTED:
            strcpy(message, "Monster Sighted!");
            break;
        default:
            sprintf(message, "City Notification %d", notificationId);
            break;
    }
    va_end(args);
    
    addGameLog("NOTIFICATION: %s", message);
    addDebugLog("Showing notification %d: %s", notificationId, message);
    
    /* NO MESSAGE BOX - just log only */
    
    /* DON'T clear immediately - let it timeout like original (30 seconds) */
}

/* Simple notification with location */
void ShowNotificationAt(int notificationId, int x, int y, ...) {
    char message[256];
    va_list args;
    
    if (!CanSendNotification(notificationId)) {
        return;
    }
    
    /* Set as active */
    activeNotificationId = notificationId;
    notificationStartTime = GetTickCount();
    
    /* Format message with location */
    va_start(args, y);
    switch (notificationId) {
        case NOTIF_EARTHQUAKE:
            sprintf(message, "Major Earthquake at (%d,%d)!", x, y);
            break;
        case NOTIF_FIRE_REPORTED:
            sprintf(message, "Fire at (%d,%d)!", x, y);
            break;
        case NOTIF_MONSTER_SIGHTED:
            sprintf(message, "Monster at (%d,%d)!", x, y);
            break;
        default:
            sprintf(message, "Alert at (%d,%d)", x, y);
            break;
    }
    va_end(args);
    
    addGameLog("NOTIFICATION: %s", message);
    addDebugLog("Showing notification %d at (%d,%d): %s", notificationId, x, y, message);
    
    /* Show dialog ONLY for emergency notifications (disasters) */
    if (notificationId >= 1000 && notificationId < 2000) {
        Notification notif;
        notif.id = notificationId;
        notif.locationX = x;
        notif.locationY = y;
        notif.hasLocation = (x >= 0 && y >= 0) ? 1 : 0;
        strcpy(notif.message, message);
        notif.timestamp = GetTickCount();
        notif.priority = 3; /* Critical for disasters */
        CreateNotificationDialog(&notif);
    }
    
    /* DON'T clear immediately - let it timeout like original (30 seconds) */
}

/* Notification dialog implementation */
static Notification* currentNotification = NULL;

void CreateNotificationDialog(Notification* notif) {
    if (!notif) return;
    
    currentNotification = notif;
    addDebugLog("Creating notification dialog for ID %d", notif->id);
    
    DialogBoxParam(GetModuleHandle(NULL), 
                   MAKEINTRESOURCE(IDD_NOTIFICATION_DIALOG),
                   hwndMain,
                   NotificationDialogProc,
                   (LPARAM)notif);
}

BOOL CALLBACK NotificationDialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Notification* notif = currentNotification;
    char titleText[256];
    char explanationText[512];
    char adviceText[512];
    char locationText[128];
    
    switch (msg) {
        case WM_INITDIALOG:
            if (!notif) return FALSE;
            
            /* Set dialog title based on notification type */
            switch (notif->id) {
                case NOTIF_EARTHQUAKE:
                    strcpy(titleText, "EARTHQUAKE DISASTER");
                    strcpy(explanationText, "A major earthquake has struck your city! Buildings have been damaged and infrastructure may be compromised.");
                    strcpy(adviceText, "Rebuild damaged areas quickly. Consider earthquake-resistant construction for the future.");
                    break;
                case NOTIF_FIRE_REPORTED:
                    strcpy(titleText, "FIRE EMERGENCY");
                    strcpy(explanationText, "A serious fire has broken out in your city! Fire departments are responding to the emergency.");
                    strcpy(adviceText, "Ensure adequate fire station coverage. Consider fireproof building materials in high-risk areas.");
                    break;
                case NOTIF_MONSTER_SIGHTED:
                    strcpy(titleText, "MONSTER ATTACK");
                    strcpy(explanationText, "A giant monster has appeared in your city! It is causing massive destruction as it moves through the area.");
                    strcpy(adviceText, "The monster will eventually leave on its own. Focus on rebuilding damaged areas afterward.");
                    break;
                default:
                    strcpy(titleText, "CITY ALERT");
                    strcpy(explanationText, "An important event has occurred in your city that requires your attention.");
                    strcpy(adviceText, "Review the situation and take appropriate action as needed.");
                    break;
            }
            
            /* Set dialog text */
            SetDlgItemText(hwnd, IDC_NOTIF_TITLE, titleText);
            SetDlgItemText(hwnd, IDC_NOTIF_MESSAGE, notif->message);
            SetDlgItemText(hwnd, IDC_NOTIF_EXPLANATION, explanationText);
            SetDlgItemText(hwnd, IDC_NOTIF_ADVICE, adviceText);
            
            /* Set location text */
            if (notif->hasLocation && notif->locationX >= 0 && notif->locationY >= 0) {
                sprintf(locationText, "Location: Sector %d, %d", notif->locationX, notif->locationY);
                SetDlgItemText(hwnd, IDC_NOTIF_LOCATION, locationText);
                EnableWindow(GetDlgItem(hwnd, IDC_GOTO_LOCATION), TRUE);
            } else {
                SetDlgItemText(hwnd, IDC_NOTIF_LOCATION, "Location: Unknown");
                EnableWindow(GetDlgItem(hwnd, IDC_GOTO_LOCATION), FALSE);
            }
            
            return TRUE;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, LOWORD(wParam));
                    return TRUE;
                    
                case IDC_GOTO_LOCATION:
                    if (notif && notif->hasLocation && notif->locationX >= 0 && notif->locationY >= 0) {
                        CenterMapOnLocation(notif->locationX, notif->locationY);
                        addGameLog("Centered map on disaster location (%d, %d)", notif->locationX, notif->locationY);
                    }
                    return TRUE;
            }
            break;
    }
    
    return FALSE;
}

void CenterMapOnLocation(int x, int y) {
    addGameLog("Centering map on (%d, %d)", x, y);
}

void addGameLogWithDialog(int showDialog, const char *format, ...) {
    va_list args;
    char buffer[512];
    
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    
    addGameLog("%s", buffer);
    
    /* NO MESSAGE BOX - disabled to prevent spam */
}