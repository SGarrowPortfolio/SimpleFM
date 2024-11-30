#include "fm.h"
#include <stdbool.h>
#include <raymath.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "miniaudio.h"
#include <raygui.h>
#include "moddedui.h"

float RunFM(FMData* data, FMOperator* a) // TODO: Make this not crash
{
    if(a->type == FM_OP_CONSTANT) // if a is a constant value
    {
        return a->data.constant;
    }
    FMWave* w = &a->data.wave;

    float freq = w->attributes[WAVE_FREQUENCY] != NO_OPERATOR ? RunFM(data, &data->operators[w->attributes[WAVE_FREQUENCY]]) : 1.0;
    float ampl = w->attributes[WAVE_AMPLITUDE] != NO_OPERATOR ? RunFM(data, &data->operators[w->attributes[WAVE_AMPLITUDE]]) : 1.0;
    float off = w->attributes[WAVE_OFFSET] != NO_OPERATOR ? RunFM(data, &data->operators[w->attributes[WAVE_OFFSET]]) : 0.0;
    if(w->attributes[WAVE_IN] == NO_OPERATOR) // If a is a root wave
    {
        return off + (ampl*G_WAVEFORMS[w->wave](freq*data->time));
    }
    return off + (ampl*G_WAVEFORMS[w->wave](freq*RunFM(data, &data->operators[w->attributes[WAVE_IN]])));
}

OperatorID AddOperator(FMData* data, Vector2 pos, FMOperator operator)
{
    if(data->opCount < data->maxOps)
    {
        data->operators[data->opCount] = operator;
        data->operators[data->opCount].id = data->opCount;
        data->operators[data->opCount].position = pos;
        data->operators[data->opCount].editMode = false;
        data->opCount++;
        return data->opCount - 1;
    }
    printf("ERROR: Cannot add operator, maxOps < opCount\n");
    exit(1);
}

// Miniaudio annoying stuff:

ma_result FMDataRead(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
    FMData* data = pDataSource;
    if (pFramesRead != NULL) 
    {
        *pFramesRead = 0;
    }

    if (frameCount == 0) 
    {
        return MA_INVALID_ARGS;
    }

    if (data == NULL) 
    {
        return MA_INVALID_ARGS;
    }

    if(pFramesOut != NULL) 
    {
        float* framesOut = pFramesOut;
        // run function
        for(int i = 0; i < frameCount; ++i) 
        {
            float sample = RunFM(data, data->head);
            data->time += data->advance;
            framesOut[i] = sample; // this will need to be refactored when I add stereo
        }
    }
    else 
    {
        data->time += (data->advance) * frameCount;
    }
    return MA_SUCCESS;
}

ma_result FMDataSeek(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    if (pDataSource == NULL) 
    {
        return MA_INVALID_ARGS;
    }
    FMData* data = pDataSource;
    data->time += data->advance * frameIndex;

    return MA_SUCCESS;
}

ma_result FMDataGetFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
    if (pDataSource == NULL) 
    {
        return MA_INVALID_ARGS;
    }
    *pFormat = ma_format_f32;
    return MA_SUCCESS;
}

ma_result FMDataGetCursor(ma_data_source* pDataSource, ma_uint64* pCursor)
{
    if (pDataSource == NULL) 
    {
        return MA_INVALID_ARGS;
    }

    FMData* data = pDataSource;
    *pCursor = (ma_uint64)(data->time / data->advance);

    return MA_SUCCESS;
}

ma_result FMDataGetLength(ma_data_source* pDataSource, ma_uint64* pLength)
{
    return MA_NOT_IMPLEMENTED;
}

static ma_data_source_vtable g_FMDataVTable = (ma_data_source_vtable){
    FMDataRead,
    FMDataSeek,
    FMDataGetFormat, 
    FMDataGetCursor, 
    FMDataGetLength 
};

FMData InitFM(float sampleRate, int maxOps) // TODO: Also do miniaudio init stuff here
{

    FMData data;
    
    // First miniaudio config

    ma_data_source_config baseConfig = ma_data_source_config_init();
    data.time = 0;
    data.advance = 1/sampleRate;
    data.operators = malloc(sizeof(FMOperator) * maxOps);
    data.maxOps = maxOps;
    data.opCount = 0;
    baseConfig.vtable = &g_FMDataVTable;

    ma_result result = ma_data_source_init(&baseConfig, &data.miniaudioData);

    return data;
}

void DrawFM(FMData* data, Camera2D camera)
{
    Color bg = GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR));
    Color border = GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
    for(int i = 0; i < data->opCount; ++i)
    {
        if(data->operators[i].type == FM_OP_WAVE)
        {
            for(int k = 0; k < WAVE_ATTRIBUTES_COUNT; ++k)
            {
                OperatorID attr = data->operators[i].data.wave.attributes[k];
                if(data->operators[i].data.wave.attributes[k] != NO_OPERATOR)
                {
                    DrawLineV((Vector2){data->operators[i].position.x + 80 + 80*k, data->operators[i].position.y + 150}, 
                            (Vector2){data->operators[attr].position.x + 200, data->operators[attr].position.y + 75}, border);
                }
            }
        }
    }
    for(int i = 0; i < data->opCount; ++i)
    {
        if(data->operators[i].type == FM_OP_WAVE)
        {



            Rectangle box = {data->operators[i].position.x, data->operators[i].position.y, 400, 150};
            DrawRectangleRec(box, bg);
            GuiGroupBox(box, "Wave");
            for(int k = 0; k < WAVE_ATTRIBUTES_COUNT; ++k)
            {
                Vector2 pos = (Vector2){data->operators[i].position.x + 80 + 80*k, data->operators[i].position.y + 150}; 
                DrawCircleV(pos, 20, bg);
                DrawCircleLinesV(pos, 20, border);

            }
            if(GuiDropdownBoxCamera(
                        (Rectangle){box.x + 100, box.y + 25, 200, 75},
                        "attentuator;sine;square;saw",
                        ((int*)&data->operators[i].data.wave.wave),
                        data->operators[i].editMode, camera)) 
                data->operators[i].editMode = !data->operators[i].editMode;
        }
        else
        {
            Rectangle box = {data->operators[i].position.x, data->operators[i].position.y, 400, 150};
            DrawRectangleRec(box, bg);
            GuiGroupBox(box, "Constant");
            if(GuiValueBoxCamera((Rectangle){box.x + 125, box.y + 25, 150, 75}, "", &data->operators[i].data.constant, -1000, 5000,data->operators[i].editMode, camera))
                data->operators[i].editMode = !data->operators[i].editMode;
        }
    }
}

void FreeFM(FMData* data) // TODO: Also do miniaudio ending stuff here
{
        data->maxOps = 0;
        ma_data_source_uninit(&data->miniaudioData);
        free(data->operators);
}
