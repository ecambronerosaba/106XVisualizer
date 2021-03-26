/*
  ==============================================================================

    CircularBuffer.h
    Created: 4 Dec 2019 2:27:17pm
    Author:  Esteban Cambronero
    Header for circular buffer that handles audio data in the visualization proccess 
  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class CircularBuffer
{
public:
    CircularBuffer(int numChannels, int size);
    void write(AudioBuffer<float> &newAudio, int start, int samples);
    void read(AudioBuffer<float> &toFill, int readSize);
private:
    int numChannels;
    int size;
    ScopedPointer<AudioBuffer<float>> audioBuffer;
    Atomic<int> head;
    Atomic<int> tail;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularBuffer);
};

