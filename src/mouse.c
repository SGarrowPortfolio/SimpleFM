#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include "mouse.h"
#include "fm.h"

void UpdateMouseData(MouseData* data, Vector2 mousePos)
{
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        data->dragging = true;
        data->collisionIdx = NO_OPERATOR;
    }
    if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) 
    {
        data->collisionIdx = NO_OPERATOR;
        data->dragging = false;
        data->panning = false;
    }
}

void UpdateUI(FMData* fmData, MouseData* mouseData, Camera2D* camera)
{
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        mouseData->dragging = true;
        mouseData->collisionIdx = NO_OPERATOR;
        for(int i = fmData->opCount-1; i >= 0; --i) // goes backwards to work well with the draw order
        {
            if(fmData->operators[i].type == FM_OP_WAVE)
            {
                for(int k = 0; k < WAVE_ATTRIBUTES_COUNT; ++k)
                {
                    Vector2 pos = (Vector2){fmData->operators[i].position.x + 80 + 80*k, fmData->operators[i].position.y + 150}; 
                    if(CheckCollisionPointCircle(mouseData->position, pos, 20)) 
                    {
                        mouseData->connection = k;
                        mouseData->collisionIdx = i;
                        fmData->operators[i].data.wave.attributes[k] = NO_OPERATOR;
                        break;
                        break;
                    }
                }
            }
            if(CheckCollisionPointRec(mouseData->position, 
            (Rectangle){fmData->operators[i].position.x, fmData->operators[i].position.y, 400, 150}))
            {
                mouseData->collisionIdx = i;
                break;
            }
        }
        if(mouseData->collisionIdx == NO_OPERATOR) mouseData->panning = true;
    }
    if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) 
    {
        if(mouseData->collisionIdx != NO_OPERATOR && mouseData->connection != FM_NO_CONNECTION)
        {
            bool connection = false;
            for(int i = fmData->opCount-1; i >= 0; --i) // goes backwards to work well with the draw order
            {
                if(i != mouseData->collisionIdx && 
                CheckCollisionPointRec(mouseData->position, (Rectangle){fmData->operators[i].position.x, fmData->operators[i].position.y, 400, 150}))
                {
                    connection = true;
                    fmData->operators[mouseData->collisionIdx].data.wave.attributes[mouseData->connection] = i;
                    break;
                }
            }
            if(!connection)
            {
                fmData->operators[mouseData->collisionIdx].data.wave.attributes[mouseData->connection] = NO_OPERATOR;
            }
        }
        mouseData->connection = FM_NO_CONNECTION;
        mouseData->collisionIdx = NO_OPERATOR;
        mouseData->dragging = false;
        mouseData->panning = false;
    }
    if(mouseData->dragging)
    {
        Vector2 delta = GetMouseDelta();
        if(mouseData->panning)
        {
            camera->offset = Vector2Add(camera->offset, delta);
        }
        else if(mouseData->collisionIdx != NO_OPERATOR && mouseData->connection == FM_NO_CONNECTION) 
        {
            fmData->operators[mouseData->collisionIdx].position = 
                Vector2Add(fmData->operators[mouseData->collisionIdx].position, delta);
        }
    }
}

