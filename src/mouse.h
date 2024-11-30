#ifndef H_MOUSE
#define H_MOUSE

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include "fm.h"

typedef enum FMConnection
{
    FM_IN,
    FM_AMPLITUDE,
    FM_FREQUENCY,
    FM_OFFSET,
    FM_NO_CONNECTION,
} FMConnection;

typedef struct MouseData {
    Vector2 position;
    FMConnection connection;
    uint16_t collisionIdx;
    bool dragging;
    bool panning;
} MouseData;

void UpdateMouseData(MouseData* data, Vector2 mousePos);
void UpdateUI(FMData* fmData, MouseData* mouseData, Camera2D* camera);

#endif
