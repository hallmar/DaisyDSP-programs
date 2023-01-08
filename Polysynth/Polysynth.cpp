/*
Basic 4 voice synthesizer that has voice allocation via USB midi and an envelopes with decay.
I made this to accompany my Monome Norns computer since I needed a simple additional polysynth for sequencing :)
Todo:
Implement a Moog ladder filter with the same envelopes that's used for the VCA
Have waveforms selectable via the two pushbuttons

Controls are:

Pot1: Envelopes amount to filter (WIP)
Pot2: Decay of envelopes
Rotary encoder: Resonance of filter (WIP)
Pushbutton1: waveshape (WIP)

*/


#include "daisy_pod.h"
#include "daisysp.h"
#include "settings.h"

using namespace daisy;
using namespace daisysp;

#define voicecount 4
#define NUM_WAVEFORMS 4

uint8_t voicenote[voicecount] = {0,0,0,0};
bool voicetrig[voicecount] = {0,0,0,0};//kind of redundant as of now since the envelopes is an AD envelopes. Not ADSR
uint8_t voicehist[voicecount] = {4, 3, 2, 1}; //use this for round robin voice allocation method
Color      colors[10];
uint8_t waveform = 0; 

Logger<LOGGER_SEMIHOST> logger;

struct envStruct
{
    AdEnv     env;
    Parameter attackParam;
    Parameter decayParam;
    Parameter curveParam;
    float     envSig;
};

struct filtStruct
{
    MoogLadder     filter;
    Parameter freqParam;
    Parameter resonanceParam;
    Parameter envFiltParam;
};

uint8_t waveforms[NUM_WAVEFORMS] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_TRI,
    Oscillator::WAVE_POLYBLEP_SAW,
    Oscillator::WAVE_POLYBLEP_SQUARE
};

DaisyPod   hw;
Oscillator osc[voicecount];
Parameter  p_envf, p_decay;

MidiUsbHandler midi; //initialize midi object
envStruct envelopes[voicecount]; // envelopes struct
filtStruct   filter; // filter struct


static void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                          AudioHandle::InterleavingOutputBuffer out,
                          size_t                                size)
{
    UpdateControls();

    for(int i = 0; i < voicecount; i++)
    {
        osc[i].SetFreq(mtof(voicenote[i]));
        osc[i].SetWaveform(waveforms[waveform]);
    }

    if(hw.button2.RisingEdge())
    {
        waveform--;
        waveform = DSY_CLAMP(waveform, 0, NUM_WAVEFORMS);
    }
    if(hw.button1.RisingEdge())
    {
        waveform++;
        waveform = DSY_CLAMP(waveform, 0, NUM_WAVEFORMS);
    }

    // Audio Loop
    for(size_t i = 0; i < size; i += 2)
    {

        
        float sig = 0;
        for(int k = 0; k < voicecount; k++)
        {
            envelopes[k].envSig = envelopes[k].env.Process();
            sig += (osc[k].Process()*envelopes[k].envSig);
            
        }

        out[i]     = sig; //left out
        out[i + 1] = sig; //right out
        
    }
    //filter.Process(out[i+1]);
    
    
}

void InitSynth(float samplerate)
{
    // Init freq Parameter to knob1 using MIDI note numbers
    // min 10, max 127, curve linear
    //p_envf.Init(hw.knob1, 0, 1, Parameter::LINEAR);
    //p_decay.Init(hw.knob2, 0, 100, Parameter::LINEAR);

    for(int i = 0; i < 4; i++)
    {
        osc[i].Init(samplerate);
        osc[i].SetAmp(0.1f);
        osc[i].SetWaveform(waveforms[waveform]);
        osc[i].SetFreq( mtof(voicenote[i]) );
    }
}

void InitEnvelopes(float samplerate)
{
    for(int i = 0; i < voicecount; i++)
    {
        //envelopes values and Init
        envelopes[i].env.Init(samplerate);
        envelopes[i].env.SetMax(1);
        envelopes[i].env.SetMin(0);
        envelopes[i].env.SetCurve(0);
        envelopes[i].attackParam.Init(hw.knob1, .01,2,Parameter::EXPONENTIAL); //assign controls to envelopes settings
        envelopes[i].decayParam.Init(hw.knob2, .01,2,Parameter::EXPONENTIAL);
    }

}

void InitColors()
{
    for(int i = 0; i < 7; i++)
    {
        colors[i].Init((Color::PresetColor)i);
    }
    colors[7].Init(1, 1, 0);
    colors[8].Init(1, 0, 1);
    colors[9].Init(0, .7, .4);
}


int main(void)
{
    float samplerate;
    
    // Init everything
    hw.Init();
    hw.SetAudioBlockSize(4);
    samplerate = hw.AudioSampleRate();
    
    MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midi_cfg);
    hw.midi.StartReceive();

    InitSynth(samplerate);
    InitEnvelopes(samplerate);
    InitColors();
    
    // start callbacks
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    //logger.StartLog(true);
    //logger.PrintLine("Logging started");

    while(1) 
    {
 	 /** Listen to MIDI for new changes */
        midi.Listen();
        hw.midi.Listen();
        /** When there are messages waiting in the queue... */
        while(midi.HasEvents())
        {
            /** Pull the oldest one from the list... */
            auto msg = midi.PopEvent();
            switch(msg.type)
            {
                case NoteOn:
                {
                    /** and change the frequency of the oscillator */
                    auto note_msg = msg.AsNoteOn();
                    //osc[0].SetFreq(mtof(note_msg.note));
                    NoteON(note_msg.note);
                    hw.UpdateLeds();
                }
                break;
                
                default: break;
            }
        }   
        while(hw.midi.HasEvents())
        {
            /** Pull the oldest one from the list... */
            auto msg = hw.midi.PopEvent();
            switch(msg.type)
            {
                case NoteOn:
                {
                    /** and change the frequency of the oscillator */
                    auto note_msg = msg.AsNoteOn();
                    //osc[0].SetFreq(mtof(note_msg.note));
                    NoteON(note_msg.note);
                    hw.UpdateLeds();
                }
                break;
        
                default: break;
            }
        }   
    }
}

void UpdateEncoder()
{
    if(hw.encoder.RisingEdge())
    {
    
    }

    //chordNum += hw.encoder.Increment();
}

void UpdateKnobs()
{
    for(int i = 0; i<voicecount;i++)
    {
        envelopes[i].env.SetTime(ADENV_SEG_ATTACK,
                                        envelopes[i].attackParam.Process());
        envelopes[i].env.SetTime(ADENV_SEG_DECAY,
                                        envelopes[i].decayParam.Process());
    }
}


void UpdateControls()
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();
    hw.UpdateLeds();
    UpdateEncoder();
    UpdateKnobs();
   
}

void NoteON(uint8_t note)
{
  
  uint8_t temp = 0;
  if (voicehist[0] == 1)
  {
    voicenote[0] = note;
    //voicetrig[0] = 1;
    //shift voicehistory by 1 to the left
    temp = voicehist[3];
    voicehist[3] = voicehist[2];
    voicehist[2] = voicehist[1];
    voicehist[1] = voicehist[0];
    voicehist[0] = temp;
    osc[0].SetFreq(mtof(note));
    envelopes[0].env.Trigger();
    hw.led1.SetColor(colors[3]);
    
    return;
  }

  if (voicehist[0] == 2)
  {
    voicenote[1] = note;
    //voicetrig[1] = 1;
    //shift voicehistory by 1 to the left
    temp = voicehist[3];
    voicehist[3] = voicehist[2];
    voicehist[2] = voicehist[1];
    voicehist[1] = voicehist[0];
    voicehist[0] = temp;
    osc[1].SetFreq(mtof(note));
    envelopes[1].env.Trigger();
    hw.led1.SetColor(colors[4]);
    return;

  }

  if (voicehist[0] == 3)
  {
    voicenote[2] = note;
    //voicetrig[2] = 1;
    //shift voicehistory by 1 to the left
    temp = voicehist[3];
    voicehist[3] = voicehist[2];
    voicehist[2] = voicehist[1];
    voicehist[1] = voicehist[0];
    voicehist[0] = temp;
    osc[2].SetFreq(mtof(note));
    envelopes[2].env.Trigger();
    hw.led1.SetColor(colors[5]);
    return;


  }

  if (voicehist[0] == 4)
  {
    voicenote[3] = note;
    //voicetrig[3] = 1;
    //shift voicehistory by 1 to the left
    temp = voicehist[3];
    voicehist[3] = voicehist[2];
    voicehist[2] = voicehist[1];
    voicehist[1] = voicehist[0];
    voicehist[0] = temp;
    osc[3].SetFreq(mtof(note));
    envelopes[3].env.Trigger();
    hw.led1.SetColor(colors[6]);
    return;

  }
}

