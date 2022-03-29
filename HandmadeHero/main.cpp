#include <windows.h>

#define internal_variable static 
#define local_persist static 
#define global_variable static 

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo = {}; // The BITMAPINFO structure defines the dimensions and color information for a DIB. 
global_variable void* BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable const unsigned char BytesPerPixel = 4;

void RenderWeirdGradient(int xOffset, int yOffset)
{
    const int pitch = BitmapWidth * BytesPerPixel;
    unsigned char* row = (unsigned char*) BitmapMemory;
    for(int y = 0; y < BitmapHeight; y++)
    {
        unsigned char* pixel = (unsigned char*) row;
        for(int x = 0; x < BitmapWidth; x++)
        {
            // Blue
            *pixel = (unsigned char) (x + xOffset);
            pixel++;

            // Green
            *pixel = (unsigned char) (y + yOffset);
            pixel++;

            // Red
            *pixel = (unsigned char) 0;
            pixel++;

            // Padding
            *pixel = (unsigned char) 0;
            pixel++;
        }
        row += pitch;
    }
}

void Win32ResizeDIBSection(long Width, long Height)
{
    if(BitmapMemory)
    {
        /*BOOL*/ VirtualFree(                       // Releases, decommits, or releases and decommits a region of pages within the virtual address space of the calling process.
            BitmapMemory, // [in] LPVOID lpAddress, // A pointer to the base address of the region of pages to be freed.
            NULL,         // [in] SIZE_T dwSize,    // The size of the region of memory to be freed, in bytes or NULL.
            MEM_RELEASE   // [in] DWORD  dwFreeType // The type of free operation.
        );
    }
    
    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize          = sizeof(BitmapInfo.bmiHeader); // The number of bytes required by the structure.
    BitmapInfo.bmiHeader.biWidth         = BitmapWidth;                  // The width of the bitmap, in pixels.
    BitmapInfo.bmiHeader.biHeight        = -BitmapHeight;                // The height of the bitmap, in pixels.
    BitmapInfo.bmiHeader.biPlanes        = 1;                            // The number of planes for the target device. This value must be set to 1.
    BitmapInfo.bmiHeader.biBitCount      = 32;                           // The number of bits-per-pixel.
    BitmapInfo.bmiHeader.biCompression   = BI_RGB;                       // The type of compression for a compressed bottom-up bitmap (top-down DIBs cannot be compressed).

    long BitmapMemorySize = BitmapWidth * BitmapHeight * BytesPerPixel;
    BitmapMemory = VirtualAlloc(                                     // Reserves, commits, or changes the state of a region of pages in the virtual address space of the calling process.
        NULL,             // [in, optional] LPVOID lpAddress,        // The starting address of the region to allocate.
        BitmapMemorySize, // [in]           SIZE_T dwSize,           // The size of the region, in bytes.
        MEM_COMMIT,       // [in]           DWORD  flAllocationType, // The type of memory allocation.
        PAGE_READWRITE    // [in]           DWORD  flProtect         // The memory protection for the region of pages to be allocated.
    );
}

void Win32UpdateWindow(HDC DeviceContext, RECT* WindowRect)
{
    int WindowHeight = WindowRect->bottom - WindowRect->top;
    int WindowWidth = WindowRect->right - WindowRect->left;
    /*int*/ StretchDIBits(
        DeviceContext,  // [in] HDC              hdc,
        0,              // [in] int              xDest,
        0,              // [in] int              yDest,
        BitmapWidth,    // [in] int              DestWidth,
        BitmapHeight,   // [in] int              DestHeight,
        0,              // [in] int              xSrc,
        0,              // [in] int              ySrc,
        WindowWidth,    // [in] int              SrcWidth,
        WindowHeight,   // [in] int              SrcHeight,
        BitmapMemory,   // [in] const VOID * lpBits,
        &BitmapInfo,    // [in] const BITMAPINFO * lpbmi,
        DIB_RGB_COLORS, // [in] UINT             iUsage,
        SRCCOPY         // [in] DWORD            rop
    );
}

LRESULT CALLBACK Win32MainWindowCallback( // Passes message information to the specified window procedure.
    HWND WindowHandle,                    // A handle to the window procedure to receive the message.
    UINT Message,                         // The message.
    WPARAM WParam,                        // Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
    LPARAM LParam                         // Additional message-specific information. The contents of this parameter depend on the value of the Msg parameter.
)
{
    LRESULT Result = 0;

    switch (Message)
    {
    case WM_SIZE:
    {
        RECT ClientRect;
        /*BOOL*/ GetClientRect(                  // Retrieves the coordinates of a window's client area.
            WindowHandle, // [in]  HWND   hWnd,  // A handle to the window whose client coordinates are to be retrieved.
            &ClientRect   // [out] LPRECT lpRect // A pointer to a RECT structure that receives the client coordinates.
        );

        Win32ResizeDIBSection(ClientRect.right, ClientRect.bottom);

        /*void*/ OutputDebugString(                               // Sends a string to the debugger for display.
            L"WM_SIZE\n" // [in, optional] LPCWSTR lpOutputString // The null-terminated string to be displayed.
        );
    } break;

    case WM_DESTROY: {
        /*void*/ OutputDebugString(
            L"WM_DESTROY\n" // [in, optional] LPCWSTR lpOutputString
        );

        Running = false;
    } break;

    case WM_CLOSE:
    {
        /*void*/ OutputDebugString(
            L"WM_CLOSE\n"
        );
        
        Running = false;
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

        RECT ClientRect;
        /*BOOL*/ GetClientRect(                  // Retrieves the coordinates of a window's client area.
            WindowHandle, // [in]  HWND   hWnd,  // A handle to the window whose client coordinates are to be retrieved.
            &ClientRect   // [out] LPRECT lpRect // A pointer to a RECT structure that receives the client coordinates.
        );

        Win32UpdateWindow(DeviceContext, &ClientRect);

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
    WNDCLASSEXW WindowClass = {};                          // Contains window class information. It is used with the RegisterClassEx and GetClassInfoEx functions.
    WindowClass.cbSize = sizeof(WNDCLASSEXW);              // The size, in bytes, of this structure.
    WindowClass.lpfnWndProc = Win32MainWindowCallback;     // The callback function that was deffined above. Windows will send messages to it.
    WindowClass.hInstance = Instance;                      // A handle to the instance that contains the window procedure for the class.
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
        return 1;
    }

    Running = true;
    MSG Message;
    int xOffset = 0;
    int yOffset = 0;
    while (Running) {
        while(PeekMessageW(&Message, NULL, NULL, NULL, PM_REMOVE))
        {
            if(Message.message == WM_QUIT)
            {
                Running = false;
            }

            /*BOOL*/ TranslateMessage(             // Translates virtual-key messages into character messages.
                &Message // [in] const MSG * lpMsg // A pointer to an MSG structure that contains message information retrieved from the calling thread's message queue by using the GetMessage or PeekMessage function.
            );

            /*LRESULT*/ DispatchMessageW(          // Dispatches a message to a window procedure.
                &Message // [in] const MSG * lpMsg // A pointer to a structure that contains the message.
            );                                     // The return value specifies the value returned by the window procedure. Although its meaning depends on the message being dispatched, the return value generally is ignored.

        }

        RenderWeirdGradient(xOffset, yOffset);

        HDC DeviceContext = GetDC(WindowHandle);
        RECT ClientRect;
        GetClientRect(WindowHandle, &ClientRect);
        Win32UpdateWindow(DeviceContext, &ClientRect);
        ReleaseDC(WindowHandle, DeviceContext);

        xOffset++;
    }

    return 0;
}