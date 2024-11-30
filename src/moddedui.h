#ifndef H_MODDEDUI
#define H_MODDEDUI

#include <raylib.h>

// Both of these functions are modified versions of functions written as part of the open-source 'raygui' library used throughout the project. 
// While the bulk of the code for these is from the raygui.h file in the 'include' folder of the project, I had to make minor adjustments to both of these functions
// Changes made detailed in 'moddedui.c'
// This use is well within the 'zlib' license used by raygui
int GuiDropdownBoxCamera(Rectangle bounds, const char *text, int *active, bool editMode, Camera2D camera);
int GuiValueBoxCamera(Rectangle bounds, const char *text, float *value, float minValue, float maxValue, bool editMode, Camera2D camera);

#endif
