#ifndef __EASY_WIN32_H__
#define __EASY_WIN32_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef EW32_BASE_NAME
#   define EW32_BASE_NAME "EasyWin32 Window"
#endif

#define uint unsigned int

#define uint8 uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define uint64 uint64_t

#define int8 int8_t
#define int16 int16_t
#define int32 int32_t
#define int64 int64_t

#define EW32_PACK_RGB(r, g, b) ((uint32)(uint8)(b) | ((uint32)((uint8)g) << 8) | ((uint32)((uint8)r) << 16))

#ifdef WIN_32
/// @brief A function type for callbacks for the WM_PAINT message
/// @param paintStruct The paint struct from BeginPaint
/// @param hdc The current device context
typedef void (func_WM_PAINT_CALLBACK)(PAINTSTRUCT* paintStruct, HDC hdc);
#else
/// @brief A function type for callbacks for the WM_PAINT message
/// @param paintStruct The paint struct from BeginPaint
/// @param hdc The current device context
typedef void (func_WM_PAINT_CALLBACK)(void* paintStruct, void* hdc);
#endif

/// @brief Input state of an EasyWIN32 tracked key
typedef enum EasyWIN32_InputState {
    EW32_INPUT_DOWN         = 0x0001, /// @brief If the key is currently down      
    EW32_INPUT_PRESSED      = 0x0003, /// @brief If the key has just been pressed

    EW32_INPUT_UP           = 0x0004, /// @brief If the key is currently up    
    EW32_INPUT_RELEASED     = 0x000C, /// @brief If the key has just been released
    
    EW32_INPUT_REPEAT       = 0x0010, /// @brief If the key is curretly repeating    
    EW32_INPUT_DOUBLE_CLICK = 0x0020, /// @brief If the key is the result of a double click
} ew32_input_state;
/// @brief Keys currently tracked
typedef enum EasyWIN32_Key {
    EW32_KEY_MOUSE_LEFT = 1,
    EW32_KEY_MOUSE_RIGHT = 2,
    EW32_KEY_MOUSE_MIDDLE = 4,
    EW32_KEY_MOUSE_X1 = 5,
    EW32_KEY_MOUSE_X2 = 6,

    EW32_KEY_0 = '0',
    EW32_KEY_1 = '1',
    EW32_KEY_2 = '2',
    EW32_KEY_3 = '3',
    EW32_KEY_4 = '4',
    EW32_KEY_5 = '5',
    EW32_KEY_6 = '6',
    EW32_KEY_7 = '7',
    EW32_KEY_8 = '8',
    EW32_KEY_9 = '9',

    EW32_KEY_A = 'A',
    EW32_KEY_B = 'B',
    EW32_KEY_C = 'C',
    EW32_KEY_D = 'D',
    EW32_KEY_E = 'E',
    EW32_KEY_F = 'F',
    EW32_KEY_G = 'G',
    EW32_KEY_H = 'H',
    EW32_KEY_I = 'I',
    EW32_KEY_J = 'J',
    EW32_KEY_K = 'K',
    EW32_KEY_L = 'L',
    EW32_KEY_M = 'M',
    EW32_KEY_N = 'N',
    EW32_KEY_O = 'O',
    EW32_KEY_P = 'P',
    EW32_KEY_Q = 'Q',
    EW32_KEY_R = 'R',
    EW32_KEY_S = 'S',
    EW32_KEY_T = 'T',
    EW32_KEY_U = 'U',
    EW32_KEY_V = 'V',
    EW32_KEY_W = 'W',
    EW32_KEY_X = 'X',
    EW32_KEY_Y = 'Y',
    EW32_KEY_Z = 'Z',

    EW32_KEY_SPACE = 500,
    EW32_KEY_ENTER,
    EW32_KEY_BACK,
    EW32_KEY_TAB,
    EW32_KEY_ESCAPE,
    EW32_KEY_ARROW_LEFT,
    EW32_KEY_ARROW_RIGHT,
    EW32_KEY_ARROW_UP,
    EW32_KEY_ARROW_DOWN,
    EW32_KEY_SHIFT,
    EW32_KEY_SHIFT_R,
    EW32_KEY_SHIFT_L,
    EW32_KEY_CTRL,
    EW32_KEY_CTRL_R,
    EW32_KEY_CTRL_L,
} ew32_key;
/// @brief The structure of a texture to blit onto the screen
typedef struct EasyWIN32_Texture {
    int width, height;
    int bitDepth; // Number of bits per pixel
    uint8* buffer;
} ew32_texture;



///// MAIN WINDOW

typedef struct EW32_InitializationParameters {
    uint width, height;
    bool doDoubleClick;
    bool doAlwaysRedrawFrame;
    bool doBilinearInterpolation;
    func_WM_PAINT_CALLBACK* wmPaintCallback;
} ew32_init_params;
/// @brief Get the default parameters for initializing the EasyWIN32 window
/// @return The default parameters
ew32_init_params EW32_GetDefaultInitParams();
/// @brief Initialize the EasyWIN32 window
/// @param windowName The name of the window
/// @param params The initialization parameters
/// @note You can get the "default parameters" by calling "EW32_GetDefaultInitParams()"
void EW32_Initilize(char* windowName, ew32_init_params params);

/// @brief Wether the EasyWIN32 window should close (for example when the user presses the [X] button at the top right)
/// @return Wether the window is marked as "shouldClose"
bool EW32_ShouldClose();
/// @brief Set the "shouldClose" state of the EasyWIN32 window
/// @param value The state to assign
void EW32_SetShouldClose(bool value);

/// @brief Get the texture used for rendering onto the screen
/// @return The texture
/// @note If you want to change the size of the texture, use "EW32_SetTexture" with the changed size
ew32_texture* EW32_textureGet();
/// @brief Replace the old render texture for a new one
/// @param texture The new texture to use
void EW32_textureSet(ew32_texture texture);

// void EW32_WindowSetFullScreen();
// void EW32_WindowMinimize();
/// @brief Get the current size of the window
/// @param x The current width
/// @param y The current height
void EW32_windowGetSize(uint* x, uint* y);
// void EW32_WindowSetSize();

/// @brief Start the frame (and set internal variables)
void EW32_StartFrame();
/// @brief End the frame (and set internal variables)
void EW32_EndFrame();

///// TIME

/// @brief Get the time elapsed between the last two frames
/// @return The time elapsed between the last two frames
double EW32_timeDelta();
/// @brief Get the average time elapsed between the last 100 consecutive frames
/// @return The smooth dt
double EW32_timeSmoothDelta();
/// @brief Get the time at the start of the current frame
/// @return The time at the start of the current frame
double EW32_timeAtFrameStart();
/// @brief Get the number of rendered frames
/// @return The number of rendered frames
uint64 EW32_timeFrameCount();

///// INPUT

/// @brief Get the state of a key
/// @param key The key to query
/// @return The state of the key
ew32_input_state EW32_inputGetKeyState(ew32_key key);
/// @brief Check wether a key has a certain state
/// @param key The key to query
/// @param state The states to check (states can be binary-or-ed together to check for multiple states)
/// @return Wether the key matches one of the wanted states
static inline bool EW32_inputIsKey(ew32_key key, ew32_input_state state) { return (EW32_inputGetKeyState(key) & state) != 0;      }
/// @brief Check wether a key is currently down
/// @param key The key to query
/// @return Wether the key is curretly down
static inline bool EW32_inputIsKeyDown(ew32_key key)                     { return  EW32_inputIsKey(key, EW32_INPUT_DOWN);         }
/// @brief Check wether a key has just been pressed
/// @param key The key to query
/// @return Wether the key has just been pressed
static inline bool EW32_inputIsKeyPressed(ew32_key key)                  { return  EW32_inputIsKey(key, EW32_INPUT_PRESSED);      }
/// @brief Check wether a key is currently up
/// @param key The key to query
/// @return Wether the key is curretly up
static inline bool EW32_inputIsKeyUp(ew32_key key)                       { return  EW32_inputIsKey(key, EW32_INPUT_DOWN);         }
/// @brief Check wether a key has just been released
/// @param key The key to query
/// @return Wether the key has just been released
static inline bool EW32_inputIsKeyReleased(ew32_key key)                 { return  EW32_inputIsKey(key, EW32_INPUT_RELEASED);     }
/// @brief Check wether a key is currently repeated
/// @param key The key to query
/// @return Wether the key is curretly repeated
static inline bool EW32_inputIsKeyRepeat(ew32_key key)                   { return  EW32_inputIsKey(key, EW32_INPUT_REPEAT);       }
/// @brief Check wether a key is the result of a double click
/// @param key The key to query
/// @return Wether the key is the result of a double click
static inline bool EW32_inputIsKeyDoubleClick(ew32_key key)              { return  EW32_inputIsKey(key, EW32_INPUT_DOUBLE_CLICK); }

#endif