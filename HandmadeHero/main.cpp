#include <windows.h>
#include <stdint.h>
#include <Xinput.h>

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return 0;
}
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return 0;
}
#define XInputSetState XInputSetState_


#define internal_variable static 
#define local_persist static 
#define global_variable static 

struct Win32WindowDimension
{
    long Width;
    long Height;
};

struct Win32BitmapBuffer
{
    BITMAPINFO Info = {};
    void* Memory;
    int Width;
    int Height;
    const uint8_t BytesPerPixel = 4;
};

global_variable bool Running;
global_variable Win32BitmapBuffer GlobalBackBuffer;
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;

static void Win32LoadXInputLibrary(void)
{
    HMODULE XInputLibrary = LoadLibraryExA(
        "xinput1_4.dll", //[in] LPCSTR lpLibFileName,
        NULL,             //HANDLE hFile,
        NULL              //[in] DWORD  dwFlags
    );

    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state*) GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state*) GetProcAddress(XInputLibrary, "XInputSetState");
    }
}

static Win32WindowDimension Win32GetWindowDimensions(HWND WindowHandle)
{
    RECT WindowRect;
    GetClientRect(WindowHandle, &WindowRect);
    Win32WindowDimension Dimensions =
    {
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top
    };
    return Dimensions;
}

void RenderWeirdGradient(Win32BitmapBuffer* backBuffer, int xOffset, int yOffset)
{
    const int pitch = backBuffer->Width * backBuffer->BytesPerPixel;
    uint8_t* row = (uint8_t*) backBuffer->Memory;
    for(int y = 0; y < backBuffer->Height; y++)
    {
        uint32_t* pixel = (uint32_t*) row;
        for(int x = 0; x < backBuffer->Width; x++)
        {
            uint8_t blue = x + xOffset;
            uint8_t green = y + yOffset;
            *pixel++ = ((green << 8) | blue);
        }
        row += pitch;
    }
}

void Win32ResizeDIBSection(Win32BitmapBuffer* backBuffer, long Width, long Height)
{
    if(backBuffer->Memory)
    {
        /*BOOL*/ VirtualFree(                             // Releases, decommits, or releases and decommits a region of pages within the virtual address space of the calling process.
            backBuffer->Memory, // [in] LPVOID lpAddress, // A pointer to the base address of the region of pages to be freed.
            NULL,               // [in] SIZE_T dwSize,    // The size of the region of memory to be freed, in bytes or NULL.
            MEM_RELEASE         // [in] DWORD  dwFreeType // The type of free operation.
        );
    }
    
    backBuffer->Width  = Width;
    backBuffer->Height = Height;

    backBuffer->Info.bmiHeader.biSize        = sizeof(backBuffer->Info.bmiHeader); // The number of bytes required by the structure.
    backBuffer->Info.bmiHeader.biWidth       = backBuffer->Width;                  // The width of the bitmap, in pixels.
    backBuffer->Info.bmiHeader.biHeight      = -backBuffer->Height;                // The height of the bitmap, in pixels.
    backBuffer->Info.bmiHeader.biPlanes      = 1;                                  // The number of planes for the target device. This value must be set to 1.
    backBuffer->Info.bmiHeader.biBitCount    = 32;                                 // The number of bits-per-pixel.
    backBuffer->Info.bmiHeader.biCompression = BI_RGB;                             // The type of compression for a compressed bottom-up bitmap (top-down DIBs cannot be compressed).

    long BitmapMemorySize = backBuffer->Width * backBuffer->Height * backBuffer->BytesPerPixel;
    backBuffer->Memory = VirtualAlloc(                                     // Reserves, commits, or changes the state of a region of pages in the virtual address space of the calling process.
        NULL,             // [in, optional] LPVOID lpAddress,              // The starting address of the region to allocate.
        BitmapMemorySize, // [in]           SIZE_T dwSize,                 // The size of the region, in bytes.
        MEM_COMMIT,       // [in]           DWORD  flAllocationType,       // The type of memory allocation.
        PAGE_READWRITE    // [in]           DWORD  flProtect               // The memory protection for the region of pages to be allocated.
    );
}

void Win32UpdateWindow(Win32BitmapBuffer* backBuffer, HDC DeviceContext, long WindowWidth, long WindowHeight)
{
    /*int*/ StretchDIBits(
        DeviceContext,     // [in] HDC              hdc,
        0,                 // [in] int              xDest,
        0,                 // [in] int              yDest,
        WindowWidth,       // [in] int              DestWidth,
        WindowHeight,      // [in] int              DestHeight,
        0,                 // [in] int              xSrc,
        0,                 // [in] int              ySrc,
        backBuffer->Width,  // [in] int              SrcWidth,
        backBuffer->Height, // [in] int              SrcHeight,
        backBuffer->Memory, // [in] const VOID * lpBits,
        &backBuffer->Info,  // [in] const BITMAPINFO * lpbmi,
        DIB_RGB_COLORS,    // [in] UINT             iUsage,
        SRCCOPY            // [in] DWORD            rop
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

        Win32WindowDimension Dimensions = Win32GetWindowDimensions(WindowHandle);
        Win32UpdateWindow(&GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);

        /*BOOL*/ EndPaint(                                    // The EndPaint function marks the end of painting in the specified window.
            WindowHandle, // [in] HWND              hWnd,     // Handle to the window that has been repainted.
            &Paint        // [in] const PAINTSTRUCT * lpPaint // Pointer to a PAINTSTRUCT structure that contains the painting information retrieved by BeginPaint.
        );
    } break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        bool WasDown = (LParam & 1 << 30) != 0;
        bool IsDown = (LParam & 1 << 31) == 0;
        if(WasDown == IsDown)
        {
            break;
        }
        
        uint32_t VKCode = WParam;
        if(VKCode == 'W')
        {
        }
        else if(VKCode == 'A')
        {
        }
        else if(VKCode == 'S')
        {
        }
        else if(VKCode == 'D')
        {
        }
        else if(VKCode == VK_UP)
        {
        }
        else if(VKCode == VK_LEFT)
        {
        }
        else if(VKCode == VK_DOWN)
        {
        }
        else if(VKCode == VK_RIGHT)
        {
        }
        else if(VKCode == VK_ESCAPE)
        {
        }
        else if(VKCode == VK_SPACE)
        {
        }
    }

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
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
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

    Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

    Running = true;
    MSG Message;
    int xOffset = 0;
    int yOffset = 0;
    Win32LoadXInputLibrary();
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

        for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ControllerIndex++)
        {
            XINPUT_STATE InputState = {};
            if(XInputGetState(ControllerIndex, &InputState) == ERROR_SUCCESS)
            {
                XINPUT_GAMEPAD* GamePad = &InputState.Gamepad;

                bool Up            = GamePad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                bool Down          = GamePad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                bool Left          = GamePad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                bool Right         = GamePad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                bool Start         = GamePad->wButtons & XINPUT_GAMEPAD_START;
                bool Back          = GamePad->wButtons & XINPUT_GAMEPAD_BACK;
                bool LeftShoulder  = GamePad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                bool RightShoulder = GamePad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                bool AButton       = GamePad->wButtons & XINPUT_GAMEPAD_A;
                bool BButton       = GamePad->wButtons & XINPUT_GAMEPAD_B;
                bool XButton       = GamePad->wButtons & XINPUT_GAMEPAD_X;
                bool YButton       = GamePad->wButtons & XINPUT_GAMEPAD_Y;

                short StickX = GamePad->sThumbLX;
                short StickY = GamePad->sThumbLY;

                xOffset += StickX >> 16;
                yOffset += StickY >> 16;
            }
        }

        RenderWeirdGradient(&GlobalBackBuffer, xOffset, yOffset);

        HDC DeviceContext = GetDC(WindowHandle);
        Win32WindowDimension Dimensions = Win32GetWindowDimensions(WindowHandle);
        Win32UpdateWindow(&GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);
        ReleaseDC(WindowHandle, DeviceContext);
    }

    return 0;
}