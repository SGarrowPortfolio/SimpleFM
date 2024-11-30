#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <stdlib.h>

// Dropdown Box control
// NOTE: Returns mouse click
// Changes made by me: Camera support
int GuiDropdownBoxCamera(Rectangle bounds, const char *text, int *active, bool editMode, Camera2D camera)
{
    int result = 0;
    GuiState state = guiState;

    int itemSelected = *active;
    int itemFocused = -1;

    // Get substrings items from text (items pointers, lengths and count)
    int itemCount = 0;
    const char **items = GuiTextSplit(text, ';', &itemCount, NULL);

    Rectangle boundsOpen = bounds;
    boundsOpen.height = (itemCount + 1)*(bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_SPACING));

    Rectangle itemBounds = bounds;

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && (editMode || !guiLocked) && (itemCount > 1) && !guiSliderDragging)
    {
        Vector2 mousePoint = GetScreenToWorld2D(GetMousePosition(), camera);

        if (editMode)
        {
            state = STATE_PRESSED;

            // Check if mouse has been pressed or released outside limits
            if (!CheckCollisionPointRec(mousePoint, boundsOpen))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) result = 1;
            }

            // Check if already selected item has been pressed again
            if (CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) result = 1;

            // Check focused and selected item
            for (int i = 0; i < itemCount; i++)
            {
                // Update item rectangle y position for next item
                itemBounds.y += (bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_SPACING));

                if (CheckCollisionPointRec(mousePoint, itemBounds))
                {
                    itemFocused = i;
                    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                    {
                        itemSelected = i;
                        result = 1;         // Item selected
                    }
                    break;
                }
            }

            itemBounds = bounds;
        }
        else
        {
            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    result = 1;
                    state = STATE_PRESSED;
                }
                else state = STATE_FOCUSED;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    if (editMode) GuiPanel(boundsOpen, NULL);

    GuiDrawRectangle(bounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), GetColor(GuiGetStyle(DROPDOWNBOX, BORDER + state*3)), GetColor(GuiGetStyle(DROPDOWNBOX, BASE + state*3)));
    GuiDrawText(items[itemSelected], GetTextBounds(DROPDOWNBOX, bounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + state*3)));

    if (editMode)
    {
        // Draw visible items
        for (int i = 0; i < itemCount; i++)
        {
            // Update item rectangle y position for next item
            itemBounds.y += (bounds.height + GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_SPACING));

            if (i == itemSelected)
            {
                GuiDrawRectangle(itemBounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_PRESSED)), GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_PRESSED)));
                GuiDrawText(items[i], GetTextBounds(DROPDOWNBOX, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_PRESSED)));
            }
            else if (i == itemFocused)
            {
                GuiDrawRectangle(itemBounds, GuiGetStyle(DROPDOWNBOX, BORDER_WIDTH), GetColor(GuiGetStyle(DROPDOWNBOX, BORDER_COLOR_FOCUSED)), GetColor(GuiGetStyle(DROPDOWNBOX, BASE_COLOR_FOCUSED)));
                GuiDrawText(items[i], GetTextBounds(DROPDOWNBOX, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_FOCUSED)));
            }
            else GuiDrawText(items[i], GetTextBounds(DROPDOWNBOX, itemBounds), GuiGetStyle(DROPDOWNBOX, TEXT_ALIGNMENT), GetColor(GuiGetStyle(DROPDOWNBOX, TEXT_COLOR_NORMAL)));
        }
    }

    // Draw arrows (using icon if available)
#if defined(RAYGUI_NO_ICONS)
    GuiDrawText("v", RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_PADDING), bounds.y + bounds.height/2 - 2, 10, 10 },
                TEXT_ALIGN_CENTER, GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))));
#else
    GuiDrawText("#120#", RAYGUI_CLITERAL(Rectangle){ bounds.x + bounds.width - GuiGetStyle(DROPDOWNBOX, ARROW_PADDING), bounds.y + bounds.height/2 - 6, 10, 10 },
                TEXT_ALIGN_CENTER, GetColor(GuiGetStyle(DROPDOWNBOX, TEXT + (state*3))));   // ICON_ARROW_DOWN_FILL
#endif
    //--------------------------------------------------------------------

    *active = itemSelected;

    // TODO: Use result to return more internal states: mouse-press out-of-bounds, mouse-press over selected-item...
    return result;   // Mouse click: result = 1
}

// Changes made by me: Camera support
// Original raygui code found here: https://github.com/raysan5/raygui/blob/master/examples/custom_input_box/custom_input_box.c 
static int guiFloatingPointIndex = 0;
int GuiValueBoxCamera(Rectangle bounds, const char *text, float *value, float minValue, float maxValue, bool editMode, Camera2D camera)
{
    #if !defined(RAYGUI_VALUEBOX_MAX_CHARS)
#define RAYGUI_VALUEBOX_MAX_CHARS  32
#endif

    int result = 0;
    GuiState state = guiState;

    char textValue[RAYGUI_VALUEBOX_MAX_CHARS + 1] = "\0";

    Rectangle textBounds = { 0 };
    if (text != NULL)
    {
        textBounds.width = (float)GetTextWidth(text) + 2;
        textBounds.height = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        textBounds.x = bounds.x + bounds.width + GuiGetStyle(VALUEBOX, TEXT_PADDING);
        textBounds.y = bounds.y + bounds.height / 2.0f - GuiGetStyle(DEFAULT, TEXT_SIZE) / 2.0f;
        if (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_LEFT) textBounds.x = bounds.x - textBounds.width - GuiGetStyle(VALUEBOX, TEXT_PADDING);
    }

    // Update control
    //--------------------------------------------------------------------
    if ((state != STATE_DISABLED) && !guiLocked && !guiSliderDragging)
    {
        Vector2 mousePoint = GetScreenToWorld2D(GetMousePosition(), camera);

        if (*value >= 0) sprintf(textValue, "+%.3f", *value);
        else sprintf(textValue, "%.3f", *value);

        bool valueHasChanged = false;

        int keyCount = (int)strlen(textValue) - guiFloatingPointIndex;

        if (editMode)
        {
            state = STATE_PRESSED;

            // Only allow keys in range [48..57]
            if (keyCount < RAYGUI_VALUEBOX_MAX_CHARS)
            {
                if (GetTextWidth(textValue) < bounds.width)
                {
                    int key = GetCharPressed();
                    if ((key >= 48) && (key <= 57) && guiFloatingPointIndex)
                    {
                        if (guiFloatingPointIndex && guiFloatingPointIndex != 4) guiFloatingPointIndex--;

                        textValue[keyCount] = (char)key;
                        textValue[++keyCount] = '\0';
                        valueHasChanged = true;
                    }
                }
            }

            // Delete text
            if (keyCount > 0)
            {
                if (IsKeyPressed(KEY_BACKSPACE))
                {
                    if (guiFloatingPointIndex < 4) guiFloatingPointIndex++;

                    keyCount--;
                    textValue[keyCount] = '\0';
                    valueHasChanged = true;
                }
            }

            // Change sign
            if (IsKeyPressed(KEY_MINUS))
            {
                if (textValue[0] == '+') textValue[0] = '-';
                else if (textValue[0] == '-') textValue[0] = '+';
                valueHasChanged = true;
            }

            // Add decimal separator
            if ((IsKeyPressed(KEY_COMMA) || IsKeyPressed(KEY_PERIOD)) && guiFloatingPointIndex == 4)
            {
                guiFloatingPointIndex--;
                valueHasChanged = true;
            }

            if (valueHasChanged)
            {
                *value = atof(textValue);
            }

            if (IsKeyPressed(KEY_ENTER) || (!CheckCollisionPointRec(mousePoint, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
            {
                guiFloatingPointIndex = 0;
                result = 1;
            }
        }
        else
        {
            if (*value > maxValue) *value = maxValue;
            else if (*value < minValue) *value = minValue;

            if (CheckCollisionPointRec(mousePoint, bounds))
            {
                state = STATE_FOCUSED;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) result = 1;
            }
        }
    }
    //--------------------------------------------------------------------

    // Draw control
    //--------------------------------------------------------------------
    Color baseColor = BLANK;
    sprintf(textValue, "%.3f", *value);

    if (state == STATE_PRESSED)
    {
        baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_PRESSED));
        textValue[(int)strlen(textValue) - guiFloatingPointIndex] = '\0';
    }
    else if (state == STATE_DISABLED) baseColor = GetColor(GuiGetStyle(VALUEBOX, BASE_COLOR_DISABLED));

    // WARNING: BLANK color does not work properly with Fade()
    GuiDrawRectangle(bounds, GuiGetStyle(VALUEBOX, BORDER_WIDTH), Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER + (state * 3))), guiAlpha), baseColor);
    GuiDrawText(textValue, GetTextBounds(VALUEBOX, bounds), TEXT_ALIGN_CENTER, Fade(GetColor(GuiGetStyle(VALUEBOX, TEXT + (state * 3))), guiAlpha));

    // Draw cursor
    if (editMode)
    {
        // NOTE: ValueBox internal text is always centered
        Rectangle cursor = { bounds.x + GetTextWidth(textValue) / 2.0f + bounds.width / 2.0f + 1, bounds.y + 2.0f * GuiGetStyle(VALUEBOX, BORDER_WIDTH), 4, bounds.height - 4 * GuiGetStyle(VALUEBOX, BORDER_WIDTH) };
        GuiDrawRectangle(cursor, 0, BLANK, Fade(GetColor(GuiGetStyle(VALUEBOX, BORDER_COLOR_PRESSED)), guiAlpha));
    }

    // Draw text label if provided
    GuiDrawText(text, textBounds, (GuiGetStyle(VALUEBOX, TEXT_ALIGNMENT) == TEXT_ALIGN_RIGHT) ? TEXT_ALIGN_LEFT : TEXT_ALIGN_RIGHT, Fade(GetColor(GuiGetStyle(LABEL, TEXT + (state * 3))), guiAlpha));
    //--------------------------------------------------------------------

    return result;
}
