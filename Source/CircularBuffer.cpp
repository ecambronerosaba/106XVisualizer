/*
  ==============================================================================

    CircularBuffer.cpp
    Created: 4 Dec 2019 2:27:17pm
    Author:  Esteban Cambronero
   Circular buffer that handles audio data in the visualization proccess 
  ==============================================================================
*/

#include "CircularBuffer.h"

/*
    Constructor for CircularBuffer that takes in the number of channels and the buffer size
 */
CircularBuffer::CircularBuffer(int channels, int bufferSize) {
    numChannels = channels;
    size = bufferSize;
    audioBuffer = new AudioBuffer<float> (numChannels, bufferSize);
    head = 0;
    tail = 0;
}

/*
    Read function for the circular buffer that outputs the read data into an audio Buffer passed by reference
 */
void CircularBuffer::read(AudioBuffer<float> &toFill, int readSize) {
    jassert(readSize < size);
    int readPos = (head.get()) - readSize;
    
    if(readPos < 0)
        readPos = size + readPos;
    for(int i = 0; i < numChannels; i++) { //only applies if it needs to loop around the buffer
        if(readPos + readSize > size - 1) {
                   int spotsLeft = size - readPos;
                   
                   toFill.copyFrom(i, 0, *(audioBuffer), i, readPos, spotsLeft);
                   toFill.copyFrom(i, spotsLeft, *(audioBuffer), i, readPos + spotsLeft, readSize - spotsLeft);
               }
               else {
                   toFill.copyFrom(i, 0, *(audioBuffer), i, readPos, readSize);
               }
       
    }
    tail += readSize;
    tail = tail.get() % size;
}

/*
    Write function for circular buffer that writes audio into the buffer from another audio buffer
 */
void CircularBuffer::write(AudioBuffer<float> &newAudio, int start, int samples) {
    for(int i = 0; i < numChannels; i++) {
        int curr = head.get();
        if(curr + samples > size - 1) {  //only applies if it needs to loop around the buffer
            int spotsLeft = size - curr;
            
            audioBuffer->copyFrom(i, curr, newAudio, i, start, samples);
            audioBuffer->copyFrom(i, 0, newAudio, i, start + spotsLeft, samples - spotsLeft);
        }
        else {
            audioBuffer->copyFrom(i, curr, newAudio, i, start, samples);
        }
        head += samples;
        head = head.get() % size;
    }
}
