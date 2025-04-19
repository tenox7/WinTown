
#ifndef _GDI_FIX
#define _GDI_FIX

/* I'm not sure where Visual C++ 2 (MSC9) falls onto this.
	I'm not sure it even matters, if you have VC 2
	You'll have access to VC 4	*/

#if(_MSC_VER < 1000)
#define WS_EX_CLIENTEDGE 	0
#define MB_ICONWARNING  	MB_ICONASTERISK
#define MB_ICONERROR    	MB_ICONSTOP

#define	DEFAULT_GUI_FONT	SYSTEM_FIXED_FONT

#define OFN_EXPLORER		0

#define	LR_LOADFROMFILE		0
#define	LR_CREATEDIBSECTION	0
#define	IMAGE_BITMAP		0
#endif


HANDLE LoadImageFromFile (LPCSTR filename, UINT fuLoad);

    // Define an alternative or provide a placeholder for older versions
    #define CHECK_MENU_RADIO_ITEM(hmenu, first, last, check, flags) \
        // Add your fallback implementation here, if necessary

#else

    // Use CheckMenuRadioItem if it's supported
    #define CHECK_MENU_RADIO_ITEM(hmenu, first, last, check, flags) \
        CheckMenuRadioItem((hmenu), (first), (last), (check), (flags))

// use original Ex calls/structures where availble
#define NEW32 1

#endif

