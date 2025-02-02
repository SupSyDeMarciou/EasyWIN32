# EasyWIN32
An interface for the Windows API which makes it much faster to start an application.

## How to use
This library depends on **GDI32**, so compile with `-lgdi32`.  

A basic program using **EasyWIN32** looks like this:
```C
#include <EasyWIN32.h>

int main() {

    // Initialize the window with default parameters
    EW32_Initilize("EasyWIN32 test window", EW32_GetDefaultInitParams());

    // ... initialization code

    while (!EW32_ShouldClose()) // Main loop
    {
        EW32_StartFrame(); // Updates input and time

        // ... Main loop code

        EW32_EndFrame();
    }

    return 0;
}
```

## Features
**EasyWIN32** provides utilities for mouse and keyboard inputs, simple time access and rendering to the screen.  
### INPUT
Inputs can be accessed using functions prefixed by `EW32_input`. Key states are stored as binary masks to allow for multiple states to be stored at once. By setting the `doDoubleClick` initialization parameter, you can have a `EW32_INPUT_DOUBLE_CLICK` state on mouse keys.
### TIME
Time can be accessed using functions prefixed by `EW32_time`. The values accessible are calculated during `EW32_StartFrame` and `EW32_EndFrame` calls, which is why they should be called in the main update loop. By setting the `doAlwaysRedrawFrame` initialization parameter, you can have the window render the frame at each `EW32_EndFrame` call.
### RENDER
You can send a texture to be rendered on screen using `EW32_textureSet` or retrieve the currently rendering texture using `EW32_textureGet`. By setting the `doBilinearInterpolation` initialization parameter, you can choose to smooth out the texture render.