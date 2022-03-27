#include <windows.h>

/*
*   Every Windows program includes an entry-point function
*   that is named either WinMain or wWinMain."
* 
*   The return value is not used by the OS, but we can use
*   the resutn value in other programs we write.
* 
*   WINAPI is the calling convention. A calling convention
*   defines how a function receives parameters from the
*   caller. For example, it defines the order that
*   parameters appear on the stack.
*
*   Use wWinMain to get the command line arguments in a Unicode string.
*   Use WinMain to get them in ANSI.
*/
int WINAPI wWinMain( 
    _In_ HINSTANCE Instance,         // The OS uses this value to identify the executable when it is in memory.
    _In_opt_ HINSTANCE PrevInstance, // This has not been used since 16-bit Windows. It will always be NULL.
    _In_ LPTSTR CommandLine,         // The command line arguments in Unicode string format.
    _In_ int ShowCode                // Flags that indicate whether the appication should be minimized, maximized, or displayed normally.
)
{
    MessageBoxExW(                                                      // Creates, displays, and operates a message box.
        NULL,                     // [in, optional] HWND    hWnd,       // A handle to the owner window of the message box to be created.
        L"This is Handmade Hero", // [in, optional] LPCWSTR lpText,     // The message to be displayed.
        L"Handmade Hero",         // [in, optional] LPCWSTR lpCaption,  // The dialog box title. If this parameter is NULL, the default title Error is used.
        MB_OK|MB_ICONINFORMATION, // [in]           UINT    uType,      // The contents and behavior of the dialog box.
        NULL                      // [in]           WORD    wLanguageId // The language for the text displayed in the message box button(s)
    ); 

    return 0;
}