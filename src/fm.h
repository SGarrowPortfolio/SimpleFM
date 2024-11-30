#ifndef H_FM
#define H_FM

#include "miniaudio.h"
#include <raylib.h>
#include <raymath.h>
#include <stdint.h>

typedef float (*WaveCallback)(float);
typedef uint16_t OperatorID;

#define NO_OPERATOR 65535

typedef enum FMOperatorType {
    FM_OP_CONSTANT,
    FM_OP_WAVE
} FMOperatorType;

typedef enum WaveformType {
    WAVEFORM_IDENTITY,
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_TYPES_COUNT
} WaveformType;

typedef enum WaveAttribute {
    WAVE_IN,
    WAVE_AMPLITUDE,
    WAVE_FREQUENCY,
    WAVE_OFFSET,
    WAVE_ATTRIBUTES_COUNT
} WaveAttribute;

typedef struct FMWave {
    WaveformType wave;
    OperatorID attributes[WAVE_ATTRIBUTES_COUNT];
} FMWave;

typedef struct FMOperator {
    union {
        FMWave wave;
        float constant;
    } data;
    FMOperatorType type;
    Vector2 position;
    OperatorID id;
    bool editMode;
} FMOperator;

typedef struct FMData {
    ma_data_source_base miniaudioData;
    FMOperator* head; // points to the head of the linked list stored in "operators"
    FMOperator* operators;
    int opCount;
    int maxOps;
    float time;
    float advance;
} FMData;

static inline FMOperator FMConstant(float value) {
    return (FMOperator){
        .type = FM_OP_CONSTANT,
        .data = {.constant = value }
    };
};

FMData InitFM(float sampleRate, int maxOps); 
ma_result FMDataRead(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
OperatorID AddOperator(FMData* data, Vector2 pos, FMOperator operator);
float RunFM(FMData* data, FMOperator* a);
void DrawFM(FMData* data, Camera2D camera); // To be implemented
void FreeFM(FMData* data);

static inline float SquareWave(float x)
{
    return sinf(x) > 0 ? 1 : -1;
}

static inline float SawtoothWave(float x)
{
    return 2 * ((x/(2*PI)) - floorf(0.5 + (x/(2*PI))));
}

static float identity(float x) { return x;}

static WaveCallback G_WAVEFORMS[WAVEFORM_TYPES_COUNT] = 
{
    identity,
    sinf, 
    SquareWave, 
    SawtoothWave
};

#endif
