#include <windows.h>

LRESULT CALLBACK MainWindowCallback( // Passes message information to the specified window procedure.
    HWND WindowHandle,               // A handle to the window procedure to receive the message.
    UINT Message,                    // The message.
    WPARAM WParam,                   // Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
    LPARAM LParam                    // Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_SIZE:
    {
        /*void*/ OutputDebugString(                               // Sends a string to the debugger for display.
            L"WM_SIZE\n" // [in, optional] LPCWSTR lpOutputString // The null-terminated string to be displayed.
        );
    } break;

    case WM_DESTROY:
    {
        /*void*/ OutputDebugString(
            L"WM_DESTROY\n" // [in, optional] LPCWSTR lpOutputString
        );
    } break;

    case WM_CLOSE:
    {
        /*void*/ OutputDebugString(
            L"WM_CLOSE\n"
        );
        // TODO: Close the window when this message is sent (will be done in a later stream).
    } break;

    case WM_ACTIVATEAPP:
    {
        /*void*/ OutputDebugString(
            L"WM_ACTIVATEAPP\n" // [in, optional] LPCWSTR lpOutputString
        );
    } break;

    case WM_PAINT:
    {
        /*void*/ OutputDebugString(
            L"WM_PAINT\n" // [in, optional] LPCWSTR lpOutputString
        );

        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(                  // This function prepares the specified window for painting and fills a PAINTSTRUCT structure with information about the painting.
            WindowHandle, // [in]  HWND          hWnd,   // Handle to the window to be repainted.
            &Paint        // [out] LPPAINTSTRUCT lpPaint // Pointer to the PAINTSTRUCT structure that will receive painting information.
        );

        /*BOOL*/ PatBlt(                                                 // The PatBlt function paints the specified rectangle using the brush that is currently selected into the specified device context.
            DeviceContext,                            // [in] HDC   hdc, // A handle to the device context.
            Paint.rcPaint.left,                       // [in] int   x,   // The x-coordinate, in logical units, of the upper-left corner of the rectangle to be filled.
            Paint.rcPaint.top,                        // [in] int   y,   // The y-coordinate, in logical units, of the upper-left corner of the rectangle to be filled.
            Paint.rcPaint.right - Paint.rcPaint.left, // [in] int   w,   // The width, in logical units, of the rectangle.
            Paint.rcPaint.bottom - Paint.rcPaint.top, // [in] int   h,   // The height, in logical units, of the rectangle.
            WHITENESS                                 // [in] DWORD rop  // The raster operation code
        );

        /*BOOL*/ EndPaint(                                    // The EndPaint function marks the end of painting in the specified window.
            WindowHandle, // [in] HWND              hWnd,     // Handle to the window that has been repainted.
            &Paint        // [in] const PAINTSTRUCT * lpPaint // Pointer to a PAINTSTRUCT structure that contains the painting information retrieved by BeginPaint.
        );
    } break;

    default:
    {
        /*void*/ OutputDebugString(
            L"default\n" // [in, optional] LPCWSTR lpOutputString
        );

        Result = DefWindowProc(
            WindowHandle, // [in] HWND   hWnd,
            Message,      // [in] UINT   Msg,
            WParam,       // [in] WPARAM wParam,
            LParam        // [in] LPARAM lParam
        );
    } break;
    }

    return Result;
}

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
    WNDCLASSEXW WindowClass = {};                        // Contains window class information. It is used with the RegisterClassEx and GetClassInfoEx functions.
    WindowClass.cbSize = sizeof(WNDCLASSEXW);       // The size, in bytes, of this structure.
    WindowClass.lpfnWndProc = MainWindowCallback;        // The callback function that was deffined above. Windows will send messages to it.
    WindowClass.hInstance = Instance;                  // A handle to the instance that contains the window procedure for the class.
    WindowClass.lpszClassName = L"HandmadHeroWindowClass"; // The class name can be any name registered with RegisterClassEx, or any of the predefined control-class names.

    ATOM RegisterClassAtom = RegisterClassExW(                // Registers a window class for subsequent use in calls to the CreateWindowEx function.
        &WindowClass // [in] const WNDCLASSEXW *unnamedParam1 // The WNDCLASSEXW that we defined above.
    );

    if (!RegisterClassAtom)
    {
        // TODO: Logging (Will be done in a later stream).
    }

    HWND WindowHandle = CreateWindowExW(                                            // Creates an overlapped, pop-up, or child window with an extended window style
        NULL,                             // [in]           DWORD     dwExStyle,    // The extended window style of the window being created.
        WindowClass.lpszClassName,        // [in, optional] LPCWSTR   lpClassName,  // A null-terminated string or a class atom created by a previous call to the RegisterClassEx function.
        L"Handmade Hero",                 // [in, optional] LPCWSTR   lpWindowName, // The window name.
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, // [in]           DWORD     dwStyle,      // The style of the window being created.
        CW_USEDEFAULT,                    // [in]           int       X,            // The initial horizontal position of the window.
        CW_USEDEFAULT,                    // [in]           int       Y,            // The initial vertical position of the window
        CW_USEDEFAULT,                    // [in]           int       nWidth,       // The width, in device units, of the window.
        CW_USEDEFAULT,                    // [in]           int       nHeight,      // The height, in device units, of the window.
        NULL,                             // [in, optional] HWND      hWndParent,   // A handle to the parent or owner window of the window being created.
        NULL,                             // [in, optional] HMENU     hMenu,        // A handle to a menu, or specifies a child-window identifier, depending on the window style.
        Instance,                         // [in, optional] HINSTANCE hInstance,    // A handle to the instance of the module to be associated with the window.
        NULL                              // [in, optional] LPVOID    lpParam       // Pointer to a value to be passed to the window through the CREATESTRUCT structure (lpCreateParams member) pointed to by the lParam param of the WM_CREATE message.
    );

    if (!WindowHandle)
    {
        // TODO: Logging (Will be done in a later stream).
    }

    MSG Message;
    for (;;) {
        BOOL MessageResult = GetMessageW(                    // Retrieves a message from the calling thread's message queue. The function dispatches incoming sent messages until a posted message is available for retrieval.
            &Message, // [out]          LPMSG lpMsg,         // A pointer to an MSG structure that receives message information from the thread's message queue.
            NULL,     // [in, optional] HWND  hWnd,          // A handle to the window whose messages are to be retrieved. The window must belong to the current thread
            NULL,     // [in]           UINT  wMsgFilterMin, // The integer value of the lowest message value to be retrieved.
            NULL      // [in]           UINT  wMsgFilterMax  // The integer value of the highest message value to be retrieved.
        );

        if (MessageResult <= 0) {
            break;
        }

        /*BOOL*/ TranslateMessage(             // Translates virtual-key messages into character messages.
            &Message // [in] const MSG * lpMsg // A pointer to an MSG structure that contains message information retrieved from the calling thread's message queue by using the GetMessage or PeekMessage function.
        );

        /*LRESULT*/ DispatchMessageW(          // Dispatches a message to a window procedure.
            &Message // [in] const MSG * lpMsg // A pointer to a structure that contains the message.
        );                                     // The return value specifies the value returned by the window procedure. Although its meaning depends on the message being dispatched, the return value generally is ignored.
    }

    return 0;
}