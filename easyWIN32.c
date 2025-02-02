#include "easyWIN32.h"
#include <sys/time.h>

#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>

#define INPUT_NB_KEYS_KEYBOARD 256 // Really 254, and even lower still if we exclude mouse, reserved and other languages
#define INPUT_NB_KEYS_MOUSE EW32_KEY_MOUSE_X2 // Left, Middle, Right, X1 and X2
typedef struct EasyWIN32_Input {

    ew32_input_state states[INPUT_NB_KEYS_KEYBOARD];
    bool shouldUpdateKeyboard;
    char text[1024]; // Text input resulting from WM_CHAR calls
    uint textLength;

    bool shouldUpdateMouse;
    int mouseX, mouseY, scroll;

} ew32_input;

static int EW32KeyToWin32(ew32_key key) {
    if (('0' <= key && key <= '9') || ('A' <= key && key <= 'Z') || key <= 6) return key;
    
    switch (key) {
        case EW32_KEY_SPACE:        return VK_SPACE;
        case EW32_KEY_TAB:          return VK_TAB;
        case EW32_KEY_ENTER:        return VK_ACCEPT;
        case EW32_KEY_BACK:         return VK_BACK;
        case EW32_KEY_ARROW_DOWN:   return VK_DOWN;
        case EW32_KEY_ARROW_UP:     return VK_UP;
        case EW32_KEY_ARROW_LEFT:   return VK_LEFT;
        case EW32_KEY_ARROW_RIGHT:  return VK_RIGHT;
        case EW32_KEY_SHIFT:        return VK_SHIFT;
        case EW32_KEY_SHIFT_L:      return VK_LSHIFT;
        case EW32_KEY_SHIFT_R:      return VK_RSHIFT;
        case EW32_KEY_CTRL:         return VK_CONTROL;
        case EW32_KEY_CTRL_L:       return VK_LCONTROL;
        case EW32_KEY_CTRL_R:       return VK_RCONTROL;
        case EW32_KEY_ESCAPE:       return VK_ESCAPE;
    
        default: fprintf(stderr, "[EasyWIN32] Key '%d' not assigned!\n", key); return 0;
    }
}

#define NB_SMOOTH_DT 128
typedef struct EasyWIN32_Time {
    double dt;
    double smoothDt;
    double timeAtFrameStart;
    
    uint64 frameCount;

    double lastDts[NB_SMOOTH_DT];
    uint lastDtIndex;
    double appStartDate;
} ew32_time;

typedef struct RenderBuffer {
    ew32_texture texture;
    BITMAPINFO header;
} render_buffer;

static struct MainWIN32 {
    const char* name;
    HWND window;
    int currentWidth, currentHeight;

    func_WM_PAINT_CALLBACK* wmPaintCallback;
    render_buffer backbuffer;
    ew32_input input;
    ew32_time time;

    bool shouldClose;
    bool alwaysRedrawframe;
    bool bilinearInterpolation;
} MAIN_W32;

bool EW32_ShouldClose() {
    return MAIN_W32.shouldClose;
}
void EW32_SetShouldClose(bool value) {
    MAIN_W32.shouldClose = value;
}

ew32_texture* EW32_textureGet() {
    return &MAIN_W32.backbuffer.texture;
}
void EW32_textureSet(ew32_texture texture) {
    free(MAIN_W32.backbuffer.texture.buffer);
    MAIN_W32.backbuffer.texture = texture;
    MAIN_W32.backbuffer.header.bmiHeader.biBitCount = texture.bitDepth;
    MAIN_W32.backbuffer.header.bmiHeader.biHeight = -texture.height;
    MAIN_W32.backbuffer.header.bmiHeader.biWidth = texture.width;
}

void EW32_windowGetSize(uint* x, uint* y) {
    *x = MAIN_W32.currentWidth;
    *y = MAIN_W32.currentHeight;
}



double EW32_timeDelta() { return MAIN_W32.time.dt; }
double EW32_timeSmoothDelta() { return MAIN_W32.time.smoothDt; }
double EW32_timeAtFrameStart() { return MAIN_W32.time.timeAtFrameStart; }
uint64 EW32_timeFrameCount() { return MAIN_W32.time.frameCount; }

static void easyWIN32_InitializeInput() {
    for (uint i = 0; i < INPUT_NB_KEYS_KEYBOARD; ++i) MAIN_W32.input.states[i] = EW32_INPUT_UP;
    MAIN_W32.input.textLength = 0;
    MAIN_W32.input.scroll = 0;
}
static void easyWIN32_UpdateInputState() {
    if (MAIN_W32.input.shouldUpdateKeyboard) for (uint i = 0; i < INPUT_NB_KEYS_KEYBOARD; ++i) MAIN_W32.input.states[i] &= (EW32_INPUT_DOWN | EW32_INPUT_UP); // Get rid of Pressed and Released states
    if (MAIN_W32.input.shouldUpdateMouse) for (uint i = 0; i < INPUT_NB_KEYS_MOUSE; ++i) MAIN_W32.input.states[i] &= (EW32_INPUT_DOWN | EW32_INPUT_UP);
    MAIN_W32.input.scroll = 0;
    MAIN_W32.input.textLength = 0;
}

ew32_input_state EW32_inputGetKeyState(ew32_key key) {
    int w32Key = EW32KeyToWin32(key);
    if (!w32Key) return 0;

    return MAIN_W32.input.states[w32Key];
}



// Callback for dealing with incomming Windows messages
LRESULT CALLBACK easyWIN32_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    LRESULT ret = 0;
    
    ///// INPUT
    ew32_input_state state = EW32_INPUT_UP;
    int mouseButton = 0;
    bool doubleClick = false;

    switch (msg) {
        case WM_SIZE: { // Window resize
            MAIN_W32.currentWidth = LOWORD(lParam);
            MAIN_W32.currentHeight = HIWORD(lParam);
        } break;

        case WM_DESTROY: { // Window getting destroyed
            PostQuitMessage(0);
        } break;

        case WM_CLOSE: { // [X] top-right button or ALT-F4
            PostQuitMessage(0);
        } break;

        case WM_ACTIVATE: { // Wether the ACTIVATED state has changed (aka. the focus has been shifted to / away from this window)
            printf("Activated!\n");

        } break;

        case WM_PAINT: { // When trying to refresh the window buffer
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(window, &paint);
            SetStretchBltMode(deviceContext, MAIN_W32.bilinearInterpolation ? STRETCH_HALFTONE : STRETCH_DELETESCANS);
            
            if (MAIN_W32.wmPaintCallback) MAIN_W32.wmPaintCallback(&paint, deviceContext);

            StretchDIBits(deviceContext,
                paint.rcPaint.left, paint.rcPaint.top,                                              // Destination pos
                paint.rcPaint.right - paint.rcPaint.left, paint.rcPaint.bottom - paint.rcPaint.top, // Destination size
                
                0, 0,                                                                               // Source pos
                MAIN_W32.backbuffer.texture.width, MAIN_W32.backbuffer.texture.height,              // Source size

                MAIN_W32.backbuffer.texture.buffer,                                                 // Source data
                (void*)&MAIN_W32.backbuffer.header,                                                 // Source bitmap header
                DIB_RGB_COLORS,                                                                     // Color mode (indexed or raw RGB)
                SRCCOPY                                                                             // Data copy mode
            );
            EndPaint(window, &paint);
        } break;

        case WM_NCMOUSEMOVE: // To access the top bar
        case WM_MOUSEMOVE: {
            MAIN_W32.input.mouseX = GET_X_LPARAM(lParam);
            MAIN_W32.input.mouseY = GET_Y_LPARAM(lParam);
        } break;

        case WM_MOUSEWHEEL: {
            MAIN_W32.input.scroll += GET_WHEEL_DELTA_WPARAM(wParam);
        } break;

        case WM_LBUTTONDBLCLK: doubleClick = true;
        case WM_LBUTTONDOWN: state = EW32_INPUT_DOWN;
        case WM_LBUTTONUP: mouseButton = VK_LBUTTON;
            goto HANDLE_MOUSE;

        case WM_MBUTTONDBLCLK: doubleClick = true;
        case WM_MBUTTONDOWN: state = EW32_INPUT_DOWN;
        case WM_MBUTTONUP: mouseButton = VK_MBUTTON;
            goto HANDLE_MOUSE;

        case WM_RBUTTONDBLCLK: doubleClick = true;
        case WM_RBUTTONDOWN: state = EW32_INPUT_DOWN;
        case WM_RBUTTONUP: mouseButton = VK_RBUTTON;
            goto HANDLE_MOUSE;
        
        case WM_XBUTTONDBLCLK: doubleClick = true;
        case WM_XBUTTONDOWN: state = EW32_INPUT_DOWN;
        case WM_XBUTTONUP: mouseButton = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2);
            goto HANDLE_MOUSE;

        HANDLE_MOUSE: {
            if (MAIN_W32.input.states[mouseButton] & EW32_INPUT_DOWN && state == EW32_INPUT_UP) {
                MAIN_W32.input.states[mouseButton] = EW32_INPUT_RELEASED;
                MAIN_W32.input.shouldUpdateMouse = true;
            }
            else if (MAIN_W32.input.states[mouseButton] & EW32_INPUT_UP && state == EW32_INPUT_DOWN) {
                MAIN_W32.input.states[mouseButton] = EW32_INPUT_PRESSED;
                MAIN_W32.input.shouldUpdateMouse = true;
            }
            else MAIN_W32.input.states[mouseButton] = state;
            if (doubleClick) MAIN_W32.input.states[mouseButton] |= EW32_INPUT_DOUBLE_CLICK;
        } break;

        case WM_KEYDOWN: state = EW32_INPUT_DOWN;
        case WM_KEYUP: {
            uint key = LOWORD(wParam);
            bool repeat = state == EW32_INPUT_DOWN && (HIWORD(lParam) & KF_REPEAT) == KF_REPEAT;

            if (MAIN_W32.input.states[key] & EW32_INPUT_DOWN && state == EW32_INPUT_UP) {
                MAIN_W32.input.states[key] = EW32_INPUT_RELEASED;
                MAIN_W32.input.shouldUpdateKeyboard = true;
            }
            else if (MAIN_W32.input.states[key] & EW32_INPUT_UP && state == EW32_INPUT_DOWN) {
                MAIN_W32.input.states[key] = EW32_INPUT_PRESSED;
                MAIN_W32.input.shouldUpdateKeyboard = true;
            }
            else MAIN_W32.input.states[key] = state;
            if (repeat) MAIN_W32.input.states[key] |= EW32_INPUT_REPEAT;
        } break;

        case WM_CHAR: // For UTF-8 or UTF-16
        case WM_DEADCHAR: // For diacritics like `
        case WM_UNICHAR: // For UTF-32
        {
            uint16 character = LOWORD(wParam);
            if (((character >> 8) & 0b11000000) == 0b10000000) {
                MAIN_W32.input.text[MAIN_W32.input.textLength++] = (uint8)(wParam >> 8);
                MAIN_W32.input.text[MAIN_W32.input.textLength++] = (uint8)wParam;
            }
            else MAIN_W32.input.text[MAIN_W32.input.textLength++] = (uint8)wParam;
            
        } break;

        default: {
            ret = DefWindowProc(window, msg, wParam, lParam);
        } break;
    }

    return ret;
}

/*
int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode) {
    
    MAIN_W32 = (struct MainWIN32) {
        .name = "EasyWIN32",
        .currentWidth = EW32_BASE_WIDTH,
        .currentHeight = EW32_BASE_HEIGHT,
        .input = {0},
        .backbuffer = {
            .texture = {
                .bitDepth = 32,
                .width = EW32_BASE_WIDTH,
                .height = EW32_BASE_HEIGHT,
                .buffer = malloc(sizeof(uint32) * EW32_BASE_HEIGHT * EW32_BASE_WIDTH),
            },
            .header = (BITMAPINFO) {
                .bmiHeader = {
                    .biSize = sizeof(MAIN_W32.backbuffer.header),
                    .biWidth = EW32_BASE_WIDTH,
                    .biHeight = -EW32_BASE_HEIGHT,
                    .biPlanes = 1,
                    .biBitCount = 32,
                    .biCompression = BI_RGB,
                    // .biSizeImage = 0,
                    // .biXPelsPerMeter = 0,
                    // .biYPelsPerMeter = 0,
                    // .biClrUsed = 0,
                    // .biClrImportant = 0
                }
            }
        },
        .shouldClose = false
    };

    WNDCLASS windowClass = {
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW, // Has own DC, redraws when changing horizontal / vertical size
        .lpfnWndProc = easyWIN32_WindowProc,
        .hInstance = instance,
        .lpszClassName = MAIN_W32.name
    };
    if (EW32_REGISTER_DOUBLE_CLICKS) windowClass.style |= CS_DBLCLKS;

    RegisterClass(&windowClass);

    MAIN_W32.window = CreateWindowEx(
        0,                              // Optional window styles.
        MAIN_W32.name,                  // Window class
        EW32_BASE_NAME,                 // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
        NULL,                           // Parent window    
        NULL,                           // Menu
        instance,                       // Instance handle
        NULL                            // Additional application data
    );
    if (!MAIN_W32.window) {
        fprintf(stderr, "[EasyWIN32] Failed to create window!\n");
        return 0; // wParam for exiting the app before statring the message loop
    }
    ShowWindow(MAIN_W32.window, showCode);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    MAIN_W32.time.appStartDate = tv.tv_sec + tv.tv_usec * 1e-6;

    return ew32_main(__argc, __argv);
}
*/

ew32_init_params EW32_GetDefaultInitParams() {
    return (ew32_init_params) {
        .width = 1920, .height = 1080,
        .doDoubleClick = true,
        .doAlwaysRedrawFrame = true,
        .doBilinearInterpolation = true,
        .wmPaintCallback = NULL
    };
}
void EW32_Initilize(char* windowName, ew32_init_params params) {
    MAIN_W32 = (struct MainWIN32) {
        .name = windowName,
        .currentWidth = params.width,
        .currentHeight = params.height,
        .input = {0},
        .backbuffer = {
            .texture = {
                .bitDepth = 32,
                .width = params.width,
                .height = params.height,
                .buffer = malloc(sizeof(uint32) * params.width * params.height),
            },
            .header = (BITMAPINFO) {
                .bmiHeader = {
                    .biSize = sizeof(MAIN_W32.backbuffer.header),
                    .biWidth = params.width,
                    .biHeight = -params.height,
                    .biPlanes = 1,
                    .biBitCount = 32,
                    .biCompression = BI_RGB,
                    // .biSizeImage = 0,
                    // .biXPelsPerMeter = 0,
                    // .biYPelsPerMeter = 0,
                    // .biClrUsed = 0,
                    // .biClrImportant = 0
                }
            }
        },
        .shouldClose = false,
        .alwaysRedrawframe = params.doAlwaysRedrawFrame,
        .bilinearInterpolation = params.doBilinearInterpolation,
        .wmPaintCallback = params.wmPaintCallback
    };

    WNDCLASS windowClass = {
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW, // Has own DC, redraws when changing horizontal / vertical size
        .lpfnWndProc = easyWIN32_WindowProc,
        .hInstance = NULL,
        .lpszClassName = MAIN_W32.name
    };
    if (params.doDoubleClick) windowClass.style |= CS_DBLCLKS;

    RegisterClass(&windowClass);

    MAIN_W32.window = CreateWindowEx(
        0,                              // Optional window styles.
        MAIN_W32.name,                  // Window class
        EW32_BASE_NAME,                 // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
        NULL,                           // Parent window    
        NULL,                           // Menu
        NULL,                           // Instance handle (NULL == current)
        NULL                            // Additional application data
    );
    if (!MAIN_W32.window) {
        fprintf(stderr, "[EasyWIN32] Failed to create window!\n");
        exit(0); // wParam for exiting the app before statring the message loop
    }
    ShowWindow(MAIN_W32.window, SW_SHOWDEFAULT);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    MAIN_W32.time.appStartDate = tv.tv_sec + tv.tv_usec * 1e-6;
}

void EW32_StartFrame() {
    MSG msg = {0};
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        
        if (msg.message == WM_QUIT) EW32_SetShouldClose(true);

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    easyWIN32_UpdateInputState();
}

void EW32_EndFrame() {
    if (MAIN_W32.alwaysRedrawframe) {
        InvalidateRect(MAIN_W32.window, NULL, FALSE);
        UpdateWindow(MAIN_W32.window);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    double newTime = tv.tv_sec + tv.tv_usec * 1e-6 - MAIN_W32.time.appStartDate;
    MAIN_W32.time.dt = newTime - MAIN_W32.time.timeAtFrameStart;
    MAIN_W32.time.timeAtFrameStart = newTime;
    MAIN_W32.time.lastDts[MAIN_W32.time.lastDtIndex = (MAIN_W32.time.lastDtIndex + 1) % NB_SMOOTH_DT] = MAIN_W32.time.dt;
    MAIN_W32.time.smoothDt = 0.0; for (uint i = 0; i < NB_SMOOTH_DT; ++i) MAIN_W32.time.smoothDt += MAIN_W32.time.lastDts[i]; MAIN_W32.time.smoothDt /= NB_SMOOTH_DT;
    ++MAIN_W32.time.frameCount;
}