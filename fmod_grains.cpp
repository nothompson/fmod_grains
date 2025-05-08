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

//fmod has fixed samplerate of 48khz
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
//--------------------------
const float FMOD_GRAIN_PARAM_GRAIN_LENGTH_MIN = 20;
const float FMOD_GRAIN_PARAM_GRAIN_LENGTH_MAX = 2000;
const float FMOD_GRAIN_PARAM_GRAIN_LENGTH_DEFAULT = 200;
//--------------------------
const float FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_MIN = 0.0;
const float FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_MAX = 1.0;
const float FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_DEFAULT = 0.0;
//--------------------------
const float FMOD_GRAIN_PARAM_GRAIN_DENSITY_MIN = 20;
const float FMOD_GRAIN_PARAM_GRAIN_DENSITY_MAX = 1000;
const float FMOD_GRAIN_PARAM_GRAIN_DENSITY_DEFAULT = 200;
//--------------------------
const float FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_MIN = 0.0;
const float FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_MAX = 1.0;
const float FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_DEFAULT = 0.0;
//--------------------------
const float FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_MIN = 0.25;
const float FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_MAX = 4.0;
const float FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_DEFAULT = 1.0;
//--------------------------
const int FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_MIN = 0;
const int FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_MAX = 2;
const int FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_DEFAULT = 0;
//--------------------------
const int FMOD_GRAIN_PARAM_GRAIN_COUNT_MIN = 1;
const int FMOD_GRAIN_PARAM_GRAIN_COUNT_MAX = 16;
const int FMOD_GRAIN_PARAM_GRAIN_COUNT_DEFAULT = 4;

#define FMOD_GRAIN_RAMPCOUNT 256;


enum
{
    //FMOD_GRAIN_PARAM_GRAIN = 0,
    FMOD_GRAIN_PARAM_GRAIN_LENGTH = 0,
    FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD,
    FMOD_GRAIN_PARAM_GRAIN_DENSITY,
    FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD,
    FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE,
    FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD,
    FMOD_GRAIN_PARAM_GRAIN_COUNT,
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

const char* spreadToggle[3] = { "off", "melodic", "textural"};

static bool                    FMOD_Grain_Running = false;
//static FMOD_DSP_PARAMETER_DESC p_grain;
static FMOD_DSP_PARAMETER_DESC p_grainLength;
static FMOD_DSP_PARAMETER_DESC p_grainLengthSpread;
static FMOD_DSP_PARAMETER_DESC p_grainDensity;
static FMOD_DSP_PARAMETER_DESC p_grainDensitySpread;
static FMOD_DSP_PARAMETER_DESC p_playback_rate;
static FMOD_DSP_PARAMETER_DESC p_playback_spread;
static FMOD_DSP_PARAMETER_DESC p_grainCount;

FMOD_DSP_PARAMETER_DESC* FMOD_Grain_dspparam[FMOD_GRAIN_NUM_PARAMETERS] =
{
    //&p_grain,
    &p_grainLength,
    &p_grainLengthSpread,
    &p_grainDensity,
    &p_grainDensitySpread,
    &p_playback_rate,
    &p_playback_spread,
    &p_grainCount
};

FMOD_DSP_DESCRIPTION FMOD_Grain_Desc =
{
    FMOD_PLUGIN_SDK_VERSION,
    "NTGrains",    // name
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
    FMOD_Grain_dspsetparamint,
    0, //FMOD_Grain_dspsetparambool,
    0, // FMOD_Gain_dspsetparamdata,
    FMOD_Grain_dspgetparamfloat,
    FMOD_Grain_dspgetparamint,
    0, //FMOD_Grain_dspgetparambool,
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
        //int params are weird, need some kind of const char array for each integer value, not very helpful for grain count
        //just use float and floor the value when sent into dsp

        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_grainLength, "length", "ms", "grain length in ms", FMOD_GRAIN_PARAM_GRAIN_LENGTH_MIN, FMOD_GRAIN_PARAM_GRAIN_LENGTH_MAX, FMOD_GRAIN_PARAM_GRAIN_LENGTH_DEFAULT);

        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_grainLengthSpread, "lengthSpread", " ", "grain length randomization", FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_MIN, FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_MAX, FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_DEFAULT);

        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_grainDensity, "trigger", "ms", "grain spawn trigger", FMOD_GRAIN_PARAM_GRAIN_DENSITY_MIN, FMOD_GRAIN_PARAM_GRAIN_DENSITY_MAX, FMOD_GRAIN_PARAM_GRAIN_DENSITY_DEFAULT);

        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_grainDensitySpread, "triggerSpread", " ", "grain trigger randomization", FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_MIN, FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_MAX, FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_DEFAULT);

        FMOD_DSP_INIT_PARAMDESC_FLOAT(p_playback_rate, "rate", " ", "playback rate", FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_MIN, FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_MAX, FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_DEFAULT);

        FMOD_DSP_INIT_PARAMDESC_INT(p_playback_spread, "rateSpread", "", "playback rate randomization", FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_MIN, FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_MAX, FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_DEFAULT, true, spreadToggle);

        FMOD_DSP_INIT_PARAMDESC_INT(p_grainCount, "maxGrains", " ", "max grain count", FMOD_GRAIN_PARAM_GRAIN_COUNT_MIN, FMOD_GRAIN_PARAM_GRAIN_COUNT_MAX, FMOD_GRAIN_PARAM_GRAIN_COUNT_DEFAULT, false, NULL);

        return &FMOD_Grain_Desc;
    }
}

class FMODGrainState
{
public:
    FMODGrainState();

    void read(float* inbuffer, float* outbuffer, unsigned int length, int channels);
    void reset();
    void setLength(float);
    float tempGrainLength() const {
        return m_target_length;
    }

    void setLengthSpread(float);
    float GrainLengthSpread() const {
        return m_target_length_spread;
    }

    void setDensity(float);
    float grainDensity() const {
        return m_target_density;
    }

    void setDensitySpread(float);
    float grainDensitySpread() const {
        return m_target_density_spread;
    }

    void setPlaybackRate(float);
    float grainPlaybackRate() const {
        return m_target_playback;
    }

    void setRateSpread(int);
    int rateSpread() const {
        return m_target_playback_spread;
    }

    void setGrainCount(int);
    int grainCount() const {
        return m_target_count;
    }

    void grainProcess(float& grains, std::vector<Granulator<float>>& granulatorChannel);

    void startGrain(std::vector<Granulator<float>>& granulatorChannel);

    CircularBuffer<float> cbL, cbR;

    //dynamic array for left and right channels. differing positions for each channel adds stereo delay/depth
    std::vector<Granulator<float>> granulatorL, granulatorR;

private:
    float m_target_grain;
    float m_current_grain;
    int   m_ramp_samples_left;

    float m_target_length;
    float m_current_length;

    float m_target_length_spread;
    float m_current_length_spread;

    float m_target_density;
    float m_current_density;

    float m_target_density_spread;
    float m_current_density_spread;

    float m_target_playback;
    float m_current_playback;

    int m_target_playback_spread;
    int m_current_playback_spread;

    float m_target_count;
    float m_current_count;

    //ms, max circular buffer length
    int maximum = 1000;

    double currentsamp = 0.0;
};

FMODGrainState::FMODGrainState() :
    //ensure they are initialized 
    m_target_length(FMOD_GRAIN_PARAM_GRAIN_LENGTH_DEFAULT),
    m_target_length_spread(FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD_DEFAULT),
    m_target_density(FMOD_GRAIN_PARAM_GRAIN_DENSITY_DEFAULT),
    m_target_density_spread(FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD_DEFAULT),
    m_target_playback(FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE_DEFAULT),
    m_target_playback_spread(FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD_DEFAULT),
    m_target_count(FMOD_GRAIN_PARAM_GRAIN_COUNT_DEFAULT)

{
    //initialize buffer with size of 1 second
    cbL.createCircularBuffer(48000);
    cbR.createCircularBuffer(48000);

    //initialize max grain limit
    granulatorL.resize(1);
    granulatorR.resize(1);

    //for each grain in granulator array, reference circular buffers 
    for (auto& grain : granulatorL) {
        grain.setBuffer(&cbL);
    }
    for (auto& grain : granulatorR) {
        grain.setBuffer(&cbR);
    }
    reset();
}

void FMODGrainState::read(float* inbuffer, float* outbuffer, unsigned int length, int channels)
{
    for (unsigned int i = 0; i < length; ++i) {
        //buffers interleaved, meaning even indices are left and odd are right
        //access by multiplying by two (number of channels)
        float inL = inbuffer[i * channels];
        //access by offsetting by one
        float inR = inbuffer[i * channels + 1];
        //now write incoming samples into buffer
        cbL.writeBuffer(inL);
        cbR.writeBuffer(inR);
        //march through time, allows for bipolar offset in grain trigger times
        currentsamp += 1.0;
        //create grains with specific parameters
        startGrain(granulatorL);
        startGrain(granulatorR);
        //initialize grains
        auto grainsL = 0.0f;
        auto grainsR = 0.0f;
        //pass in grains and granulators to iterate through them and mix multiple grain voices
        grainProcess(grainsL, granulatorL);
        grainProcess(grainsR, granulatorR);
        //normalize by values, tried making seperate function but was being weird this is fine idk
        grainsL /= pow(static_cast<float>(granulatorL.size()), 0.5);
        grainsR /= pow(static_cast<float>(granulatorR.size()), 0.5);
        //what you hear
        outbuffer[i * channels] = grainsL;
        outbuffer[i * channels + 1] = grainsR;
    }
}

void FMODGrainState::reset()
{
    //fill buffers with 0s
    cbL.flushBuffer(); 
    cbR.flushBuffer();

    /*
    m_current_length = m_target_length;
    m_current_length_spread = m_target_length_spread;
    m_current_density = m_target_density;
    m_current_density_spread = m_target_density_spread;
    m_current_playback = m_target_playback;
    m_current_playback_spread = m_target_playback_spread;
    */
    m_ramp_samples_left = 0;
    //currentsamp = 0.0;
}

void FMODGrainState::startGrain(std::vector<Granulator<float>>& granulatorChannel) {
    for (int i = 0; i < granulatorChannel.size(); i++) {

        if (granulatorChannel[i].grainTrigger() == true) {

            //random number wrapped to maximum. with a buffer size of 1 second, we get random numbers between 0 and 999
            auto grainPosition = rand() % maximum;

            auto lengthspread = m_current_length_spread;

            auto randspread = (lengthspread * 0.5) + (rand() / (double)RAND_MAX) * (lengthspread);

            auto myGrainLength = m_current_length + (randspread * lengthspread);

            //0.5 to 4.0 
            auto texturalSpread = 0.5 + (rand() / (double)RAND_MAX) * (3.5);

            const double spreadTable[] = { static_cast<double>(m_current_playback) * -0.5, 0.0, static_cast<double>(m_current_playback), static_cast<double>(m_current_playback) * 3.0 };
            auto spreadIndex = rand() % 4;

            auto melodicSpread = spreadTable[spreadIndex];

            double playbackrate = 0.0;

            if (m_current_playback_spread <= 1) {
                playbackrate = m_current_playback + (melodicSpread * static_cast<double>(m_current_playback_spread));
            }

            if (m_current_playback_spread >= 2) {
                playbackrate = m_current_playback * (texturalSpread);
            }

            auto densityspread = (rand() / (double)RAND_MAX * m_current_density) - (m_current_density * 0.5);

            auto trigOffset = densityspread * m_current_density_spread;

            //intialize with parameters 
            granulatorChannel[i].startGrain(static_cast<double>(myGrainLength), static_cast<double>(grainPosition), currentsamp, trigOffset, playbackrate);
            granulatorChannel[i].setTrigger(m_current_density);
        }
        else {
            granulatorChannel[i].triggerTick();
        }
    }
}

void FMODGrainState::grainProcess(float& grains, std::vector<Granulator<float>>& granulatorChannel) {

    for (auto& grain : granulatorChannel) {
        //double playbackrate = m_current_playback + (spread * m_current_playback_spread);
        grains += (grain.processGrain(currentsamp));
        grain.deleteGrains();
    }
}

void FMODGrainState::setLength(float length) {
    m_current_length = length;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}

void FMODGrainState::setLengthSpread(float lengthspread) {
    m_current_length_spread = lengthspread;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}


void FMODGrainState::setDensity(float density) {
    m_current_density = density;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}

void FMODGrainState::setDensitySpread(float spread) {
    m_current_density_spread = spread;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}

void FMODGrainState::setPlaybackRate(float rate) {
    m_current_playback = rate;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}

void FMODGrainState::setRateSpread(int spread) {
    m_current_playback_spread = spread;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;
}

void FMODGrainState::setGrainCount(int count) {
    m_current_count = count;
    m_ramp_samples_left = FMOD_GRAIN_RAMPCOUNT;

    auto grainMax = floor(m_current_count);

    //resize dynamic vector on change in grain count param

    granulatorL.resize(grainMax);

    granulatorR.resize(grainMax);

    //each grain needs to reference circular buffers for input
    for (auto& grain : granulatorL) {
        grain.setBuffer(&cbL);
    }
    for (auto& grain : granulatorR) {
        grain.setBuffer(&cbR);
    }
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
    case FMOD_GRAIN_PARAM_GRAIN_LENGTH:
        state->setLength(value);
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD:
        state->setLengthSpread(value);
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_DENSITY:
        state->setDensity(value);
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD:
        state->setDensitySpread(value);
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE:
        state->setPlaybackRate(value);
        return FMOD_OK;
    }


    

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_dspgetparamfloat(FMOD_DSP_STATE* dsp_state, int index, float* value, char* valuestr)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    
    switch (index)
    {
    case FMOD_GRAIN_PARAM_GRAIN_LENGTH:
        *value = state->tempGrainLength();
        //if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, "%.1f dB", state->tempGrainLength());
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_LENGTH_SPREAD:
        *value = state->GrainLengthSpread();
        //if (valuestr) snprintf(valuestr, FMOD_DSP_GETPARAM_VALUESTR_LENGTH, "%.1f dB", state->tempGrainLength());
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_DENSITY:
        *value = state->grainDensity();
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_DENSITY_SPREAD:
        *value = state->grainDensitySpread();
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_RATE:
        *value = state->grainPlaybackRate();
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_COUNT:
        *value = state->grainCount();
        return FMOD_OK;
    }
    

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_dspsetparamint(FMOD_DSP_STATE* dsp_state, int index, int value)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD:
        state->setRateSpread(value);
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_COUNT:
        state->setGrainCount(value);
        return FMOD_OK;
    }

    return FMOD_ERR_INVALID_PARAM;
}

FMOD_RESULT F_CALL FMOD_Grain_dspgetparamint(FMOD_DSP_STATE* dsp_state, int index, int* value, char* valuestr)
{
    FMODGrainState* state = (FMODGrainState*)dsp_state->plugindata;

    switch (index)
    {
    case FMOD_GRAIN_PARAM_GRAIN_PLAYBACK_SPREAD:
        *value = state->rateSpread();
        return FMOD_OK;
    case FMOD_GRAIN_PARAM_GRAIN_COUNT:
        *value = state->grainCount();
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
