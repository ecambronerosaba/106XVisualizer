/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "CircularBuffer.h"
#include "SineVisualizer.h"
#include "CircularMesh.h"
//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent, public ChangeListener, public Button::Listener

{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;
    
    void buttonClicked(Button *buttonClicked) override;

private:
    //==============================================================================
    // Your private member variables go here...
    enum AudioState {
        STOPPED,
        STARTING,
        PLAYING,
        PAUSING,
        PAUSED,
        STOPPING
    };
    bool audioFileEnabled;
    
    //GUI BUTTONS
    TextButton openFileButton;
    TextButton playButton;
    TextButton stopButton;
    
    TextButton twoDButton;
    TextButton threeDButton;
    TextButton lineVisualizer;
    TextButton squareVisualizer;
    TextButton triangleVisualizer;
    
    AudioDeviceSelectorComponent audioIOSelector;
    
    //Audio Reading Variables
    AudioFormatManager manager;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    AudioTransportSource audioSource;
    AudioState state;
    
    //Circular Buffer
    CircularBuffer *circBuffer;
    
    //Visualizers
    SineVisualizer *twoDVisualizer;
    CircularMesh *circMesh;
    CircularMesh *lineMesh;
    CircularMesh *triangleMesh;
    CircularMesh *squareMesh;
    
    void changeAudioState (MainComponent::AudioState newState);
    void openFile();
    void play();
    void stop();
    void twoDButtonClicked(Button *&buttonClicked);
    void circVisualizerClicked(Button *&buttonClicked);
    void lineVisualizerClicked(Button *&buttonClicked);
    void squareVisualizerClicked(Button *&buttonClicked);
    void triangleVisualizerClicked(Button *&buttonClicked);
    void setupGUI(Listener *mainComponent);
    void resizeButtons(int bWidth, int bHeight, int bMargins);
    void changeListenerCallback(ChangeBroadcaster *source) override;
    void resizeVisualizers(int width, int height);
    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
