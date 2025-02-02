#ifndef __EASY_WIN32_H__
#define __EASY_WIN32_H__

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define EW32_REGISTER_DOUBLE_CLICKS true
#define EW32_ALWAYS_REDRAW_FRAME true
#define EW32_BASE_WIDTH 1920
#define EW32_BASE_HEIGHT 1080
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

int ew32_main(int argc, char** argv);

typedef enum EasyWIN32_InputState {
    EW32_INPUT_DOWN = 0x0001,
    EW32_INPUT_PRESSED = 0x0003,

    EW32_INPUT_UP = 0x0004,
    EW32_INPUT_RELEASED = 0x000C,
    
    EW32_INPUT_REPEAT = 0x0010,
    EW32_INPUT_DOUBLE_CLICK = 0x0020,
} ew32_input_state;
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

typedef struct EasyWIN32_Texture {
    int width, height;
    int bitDepth; // Number of bits per pixel
    uint8* buffer;
} ew32_texture;



///// MAIN WINDOW
bool EW32_ShouldClose();
void EW32_SetShouldClose(bool value);

ew32_texture* EW32_GetTexture();
void EW32_SetTexture(ew32_texture texture);

// void EW32_WindowSetFullScreen();
// void EW32_WindowMinimize();

void EW32_WindowGetSize(uint* x, uint* y);
// void EW32_WindowSetSize();

void EW32_StartFrame();
void EW32_EndFrame();

///// TIME
double EW32_GetDeltaTime();
double EW32_GetSmoothDeltaTime();
double EW32_GetTimeAtFrameStart();
uint64 EW32_GetFrameCount();

///// INPUT
ew32_input_state EW32_inputGetKeyState(ew32_key key);
static inline bool EW32_inputIsKey(ew32_key key, ew32_input_state state) { return (EW32_inputGetKeyState(key) & state) != 0;      }
static inline bool EW32_inputIsKeyDown(ew32_key key)                     { return  EW32_inputIsKey(key, EW32_INPUT_DOWN);         }
static inline bool EW32_inputIsKeyPressed(ew32_key key)                  { return  EW32_inputIsKey(key, EW32_INPUT_PRESSED);      }
static inline bool EW32_inputIsKeyUp(ew32_key key)                       { return  EW32_inputIsKey(key, EW32_INPUT_DOWN);         }
static inline bool EW32_inputIsKeyReleased(ew32_key key)                 { return  EW32_inputIsKey(key, EW32_INPUT_RELEASED);     }
static inline bool EW32_inputIsKeyRepeat(ew32_key key)                   { return  EW32_inputIsKey(key, EW32_INPUT_REPEAT);       }
static inline bool EW32_inputIsKeyDoubleClick(ew32_key key)              { return  EW32_inputIsKey(key, EW32_INPUT_DOUBLE_CLICK); }

#endif