/*==============================================================================
Gain DSP Plugin Example
Copyright (c), Firelight Technologies Pty, Ltd 2004-2025.

This example shows how to create a simple gain DSP effect.
==============================================================================*/

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "grainHeader.h"

const int sampleRate = 48000;
const double PI = 3.141592653589793;

#define FMOD_GRAIN_USEPROCESSCALLBACK            /* FMOD plugins have 2 methods of processing data.  
                                                    1. via a 'read' callback which is compatible with FMOD Ex but limited in functionality, or
                                                    2. via a 'process' callback which exposes more functionality, like masks and query before process early out logic. */

extern "C" {
    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

const float FMOD_GRAIN_PARAM_GRAIN_MIN = -80.0f;
const float FMOD_GRAIN_PARAM_GRAIN_MAX = 10.0f;
const float FMOD_GRAIN_PARAM_GRAIN_DEFAULT = 0.0f;
#define FMOD_GRAIN_RAMPCOUNT 256;


enum
{
    FMOD_GRAIN_PARAM_GRAIN = 0,
    FMOD_GRAIN_NUM_PARAMETERS
};

#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= FMOD_GRAIN_PARAM_GRAIN_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? FMOD_GRAIN_PARAM_GRAIN_MIN : 20.0f * log10f((float)__linval__))

FMOD_RESULT F_CALL FMOD_Grain_dspcreate(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_Grain_dsprelease(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_Grain_dspreset(FMOD_DSP_STATE* dsp_state);
#ifdef FMOD_GRAIN_USEPROCESSCALLBACK
FMOD_RESULT F_CALL FMOD_Grain_dspprocess(FMOD_DSP_STATE* dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY* inbufferarray, FMOD_DSP_BUFFER_ARRAY* outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
#else
FMOD_RESULT F_CALL FMOD_Grain_dspread(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels);
#endif
FMOD_RESULT F_CALL FMOD_Grain_dspsetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float value);
FMOD_RESULT F_CALL FMOD_Grain_dspsetparamint(FMOD_DSP_STATE* dsp_state, int index, int value);
FMOD_RESULT F_CALL FMOD_Grain_dspsetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL value);
FMOD_RESULT F_CALL FMOD_Grain_dspsetparamdata(FMOD_DSP_STATE* dsp_state, int index, void* data, unsigned int length);
FMOD_RESULT F_CALL FMOD_Grain_dspgetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_Grain_dspgetparamint(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_Grain_dspgetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL* value, char* valuestr);
FMOD_RESULT F_CALL FMOD_Grain_dspgetparamdata(FMOD_DSP_STATE* dsp_state, int index, void** value, unsigned int* length, char* valuestr);
FMOD_RESULT F_CALL FMOD_Grain_shouldiprocess(FMOD_DSP_STATE* dsp_state, FMOD_BOOL inputsidle, unsigned int length, FMOD_CHANNELMASK inmask, int inchannels, FMOD_SPEAKERMODE speakermode);
FMOD_RESULT F_CALL FMOD_Grain_sys_register(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_Grain_sys_deregister(FMOD_DSP_STATE* dsp_state);
FMOD_RESULT F_CALL FMOD_Grain_sys_mix(FMOD_DSP_STATE* dsp_state, int stage);

static bool                    FMOD_Grain_Running = false;
static FMOD_DSP_PARAMETER_DESC p_grain;

FMOD_DSP_PARAMETER_DESC* FMOD_Grain_dspparam[FMOD_GRAIN_NUM_PARAMETERS] =
{
    &p_grain
};

FMOD_DSP_DESCRIPTION FMOD_Grain_Desc =
{
    FMOD_PLUGIN_SDK_VERSION,
    "myGrains",    // name
    0x00010000,     // plug-in version
    1,              // number of input buffers to process
    1,              // number of output buffers to process
    FMOD_Grain_dspcreate,
    FMOD_Grain_dsprelease,
    FMOD_Grain_dspreset,
#ifndef FMOD_GRAIN_USEPROCESSCALLBACK
    FMOD_Grain_dspread,
#else
    0,
#endif
#ifdef FMOD_GRAIN_USEPROCESSCALLBACK
    FMOD_Grain_dspprocess,
#else
    0,
#endif
    0,
    FMOD_GRAIN_NUM_PARAMETERS,
    FMOD_Grain_dspparam,
    FMOD_Grain_dspsetparamfloat,
    0, // FMOD_Gain_dspsetparamint,
    FMOD_Grain_dspsetparambool,
    0, // FMOD_Gain_dspsetparamdata,
    FMOD_Grain_dspgetparamfloat,
    0, // FMOD_Gain_dspgetparamint,
    FMOD_Grain_dspgetparambool,
    0, // FMOD_Gain_dspgetparamdata,
    FMOD_Grain_shouldiprocess,
    0,                                      // userdata
    FMOD_Grain_sys_register,
    FMOD_Grain_sys_deregister,
    FMOD_Grain_sys_mix
};

extern "C"
{

    F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
    {
        static float grain_mapping_values[] = { -80, -50, -30, -10, 10 };
        static float grain_mapping_scale[] = { 0, 2, 4, 7, 11 };

        FMOD_DSP_INIT_PARAMDESC_FLOAT_WITH_MAPPING(p_grain, "Grain", "dB", "Grain in dB. -80 to 10. Default = 0", FMOD_GRAIN_PARAM_GRAIN_DEFAULT, grain_mapping_values, grain_mapping_scale);
        return &FMOD_Grain_Desc;
    }

}

class FMODGrainState
{
public:
    FMODGrainState();

    void read(float* inbuffer, float* outbuffer, unsigned int length, int channels);
    void reset();
    void setGrain(float);
    float grain() const { return LINEAR_TO_DECIBELS(m_target_grain); }
    CircularBuffer<float> cbL, cbR;
 

private:
    float m_target_grain;
    float m_current_grain;
    int   m_ramp_samples_left;
};

FMODGrainState::FMODGrainState()
{
    m_target_grain = DECIBELS_TO_LINEAR(FMOD_GRAIN_PARAM_GRAIN_DEFAULT);

    //initialize buffer with size of 1 second
    cbL.createCircularBuffer(48000);
    cbR.createCircularBuffer(48000);

    reset();
}

void FMODGrainState::read(float* inbuffer, float* outbuffer, unsigned int length, int channels)
{
    // Note: buffers are interleaved

    for (unsigned int i = 0; i < length; ++i) {

        //buffers interleaved, meaning even indices are left and odd are right
        //access by multiplying by two 
        float inL = inbuffer[i * channels];
        //access by offsetting by one
        float inR = inbuffer[i * channels + 1];

        //now write incoming samples into buffer
        
        
        float delayL = cbL.readBuffer(sampleRate);
        float delayR = cbR.readBuffer(sampleRate);

        float echoL = inL + 0.75 * delayL;
        float echoR = inR + 0.75 * delayR;

        cbL.writeBuffer(echoL);
        cbR.writeBuffer(echoR);
        

        double hann = 0.5 * (1.0 - cos(2.0 * PI * i / (length - 1)));

        outbuffer[i * channels] = echoL;
        outbuffer[i * channels + 1] = echoR;
    }
}

void FMODGrainState::reset()
{
    //fill buffers with 0s
    cbL.flushBuffer(); 
    cbR.flushBuffer();

    m_current_grain = m_target_grain;
    m_ramp_samples_left = 0;
}

void FMODGrainState::setGrain(float grain)
{
    m_target_grain = DECIBELS_TO_LINEAR(grain);
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}

FMOD_RESULT F_CALL FMOD_Grain_dspcreate(FMOD_DSP_STATE* dsp_state)
{

    void* mem = FMOD_DSP_ALLOC(dsp_state, sizeof(FMODGrainState));
    if (!mem) return FMOD_ERR_MEMORY;

    FMODGrainState* state = new (mem) FMODGrainState();
    //explicit new call, needs an explicit destructor
    dsp_state->plugindata = state;



    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Grain_dsprelease(FMOD_DSP_STATE* dsp_state)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;
    state->~FMODGrainState();
    //explicit destructor
    FMOD_DSP_FREE(dsp_state, state);
    return FMOD_OK;

}

#ifdef FMOD_GRAIN_USEPROCESSCALLBACK

FMOD_RESULT F_CALL FMOD_Grain_dspprocess(FMOD_DSP_STATE* dsp_state, unsigned int length, const FMOD_DSP_BUFFER_ARRAY* inbufferarray, FMOD_DSP_BUFFER_ARRAY* outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    if (op == FMOD_DSP_PROCESS_QUERY)
    {
        if (outbufferarray && inbufferarray)
        {
            outbufferarray[0].buffernumchannels[0] = inbufferarray[0].buffernumchannels[0];
            outbufferarray[0].speakermode = inbufferarray[0].speakermode;
        }

        if (inputsidle)
        {
            return FMOD_ERR_DSP_DONTPROCESS;
        }
    }
    else
    {
        state->read(inbufferarray[0].buffers[0], outbufferarray[0].buffers[0], length, inbufferarray[0].buffernumchannels[0]); // input and output channels count match for this effect
    }

    return FMOD_OK;
}

#else

FMOD_RESULT F_CALL FMOD_Grain_dspread(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* /*outchannels*/)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;
    state->read(inbuffer, outbuffer, length, inchannels); // input and output channels count match for this effect
    return FMOD_OK;
}

#endif

FMOD_RESULT F_CALL FMOD_Grain_dspreset(FMOD_DSP_STATE* dsp_state)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;
    state->reset();
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Grain_dspsetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float value)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GRAIN_PARAM_GRAIN:
        state->setGrain(value);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_dspgetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GRAIN_PARAM_GRAIN:
        *value = state->grain();
        if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, "%.1f dB", state->grain());
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_dspsetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL value)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_dspgetparambool(FMOD_DSP_STATE* dsp_state, int index, FMOD_BOOL* value, char* valuestr)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_shouldiprocess(FMOD_DSP_STATE* /*dsp_state*/, FMOD_BOOL inputsidle, unsigned int /*length*/, FMOD_CHANNELMASK /*inmask*/, int /*inchannels*/, FMOD_SPEAKERMODE /*speakermode*/)
{
    if (inputsidle)
    {
        return FMOD_ERR_DSP_DONTPROCESS;
    }

    return FMOD_OK;
}


FMOD_RESULT F_CALL FMOD_Grain_sys_register(FMOD_DSP_STATE* /*dsp_state*/)
{
    FMOD_Grain_Running = true;
    // called once for this type of dsp being loaded or registered (it is not per instance)
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Grain_sys_deregister(FMOD_DSP_STATE* /*dsp_state*/)
{
    FMOD_Grain_Running = false;
    // called once for this type of dsp being unloaded or de-registered (it is not per instance)
    return FMOD_OK;
}

FMOD_RESULT F_CALL FMOD_Grain_sys_mix(FMOD_DSP_STATE* /*dsp_state*/, int /*stage*/)
{
    // stage == 0 , before all dsps are processed/mixed, this callback is called once for this type.
    // stage == 1 , after all dsps are processed/mixed, this callback is called once for this type.
    return FMOD_OK;
}
