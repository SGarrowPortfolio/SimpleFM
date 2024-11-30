#include "fm.h"
#include "mouse.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <raygui.h>


#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
#endif


const int screenWidth = 1280;
const int screenHeight = 720;
const int gridSize = 120;
const int maxOperators = 256;

Camera2D camera;
FMData fmData;
Rectangle box = (Rectangle){400, 200, 400, 150};
MouseData mouseData;

void UpdateDrawFrame(void);     

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    FMData* data;

    data = (FMData*)pDevice->pUserData;
    FMDataRead(data, pOutput, frameCount, NULL);

    (void)pInput;   /* Unused. */
}


int main(void)
{

    camera = (Camera2D){
    .zoom = 1.0,
    .rotation = 0,
    .offset = (Vector2){0,0},
    .target = (Vector2){0,0}
    };

    mouseData = (MouseData){
        .collisionIdx = NO_OPERATOR,
        .connection = FM_NO_CONNECTION,
        .panning = false,
        .dragging = false,
        .position = Vector2Zero()
    };

    ma_device device;



    ma_device_config deviceConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.sampleRate        = 44100;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &fmData;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) 
    {
        printf("Failed to open playback device.\n");
        return 4;
    }

    printf("Device Name: %s\n", device.playback.name);

    fmData = InitFM(44100, maxOperators);
    AddOperator(&fmData, (Vector2){screenWidth/2.0, screenHeight/2.0}, (FMOperator){
            .type= FM_OP_WAVE,
            .data = {
                .wave = 
                {
                    .wave = WAVEFORM_IDENTITY,
                    .attributes = {
                        NO_OPERATOR,
                        NO_OPERATOR,
                        NO_OPERATOR,
                        NO_OPERATOR
                    }
                }
            }
            });
    fmData.head = &fmData.operators[fmData.opCount-1];

    if (ma_device_start(&device) != MA_SUCCESS) 
    {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return -5;
    }

    InitWindow(screenWidth, screenHeight, "FM Synth");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

    #if defined(__EMSCRIPTEN__)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
    SetTargetFPS(60);   

    while (!WindowShouldClose())    
    {
        UpdateDrawFrame();
    }
#endif
    FreeFM(&fmData);
    ma_device_uninit(&device);
    CloseWindow();        
    return 0;
}

void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    mouseData.position = GetScreenToWorld2D(GetMousePosition(), camera);
    if(IsKeyPressed(KEY_C)) AddOperator(&fmData, (Vector2){mouseData.position.x - 200, mouseData.position.y - 75}, FMConstant(1.0));
    if(IsKeyPressed(KEY_W)) AddOperator(&fmData, (Vector2){mouseData.position.x - 200, mouseData.position.y - 75}, (FMOperator){
            .type = FM_OP_WAVE,
            .data = {
                .wave = (FMWave){
                    .wave = WAVEFORM_SINE,
                    .attributes = {
                        NO_OPERATOR,
                        NO_OPERATOR,
                        NO_OPERATOR,
                        NO_OPERATOR
                    }
                }
            }
            });

    UpdateUI(&fmData, &mouseData, &camera);
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode2D(camera);
        Vector2 v =  (Vector2){
            camera.offset.x - gridSize*floor(camera.offset.x/gridSize),
                camera.offset.y - gridSize*floor(camera.offset.y/gridSize)};
        GuiGrid((Rectangle){
                -gridSize-camera.offset.x+v.x, -gridSize-camera.offset.y+v.y,
                screenWidth + gridSize, screenHeight + gridSize}
                , "", gridSize, gridSize, NULL);
        DrawFM(&fmData, camera);
        if(mouseData.dragging && mouseData.collisionIdx != NO_OPERATOR && mouseData.connection != FM_NO_CONNECTION)
        {
            Vector2 pos = (Vector2){fmData.operators[mouseData.collisionIdx].position.x + 80 + 80*mouseData.connection, fmData.operators[mouseData.collisionIdx].position.y + 150}; 
            DrawLineV(pos, mouseData.position, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));
        }
    EndMode2D();
    EndDrawing();
}
