/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"


//==============================================================================
MainComponent::MainComponent() : audioIOSelector(deviceManager, 1, 2, 0, 0, false, false, true, true)
{
    audioFileEnabled = false;
    
    //Audio Setup
    state = AudioState::STOPPED;
    manager.registerBasicFormats();
    audioSource.addChangeListener(this);
    setAudioChannels(2, 2);
    
    //GUI Setup
    setupGUI(this);
    
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (2, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    audioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    
    circBuffer = new CircularBuffer(2, samplesPerBlockExpected*10);
    
    twoDVisualizer = new SineVisualizer(circBuffer);
    addChildComponent(twoDVisualizer);
    
    circMesh = new CircularMesh(circBuffer, "circle");
    addChildComponent(circMesh);
    
    lineMesh = new CircularMesh(circBuffer, "line");
    addChildComponent(lineMesh);
    
    triangleMesh = new CircularMesh(circBuffer, "triangle");
    addChildComponent(triangleMesh);
    
    squareMesh = new CircularMesh(circBuffer, "square");
    addChildComponent(squareMesh);
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    if(audioFileEnabled)
        audioSource.getNextAudioBlock(bufferToFill);
    else {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    
    //Writing to Circular Buffer
    circBuffer->write(*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples);
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    if (lineMesh!= nullptr)
      {
          lineMesh->stop();
          removeChildComponent (lineMesh);
          delete lineMesh;
      }
      
      if (circMesh != nullptr)
      {
          circMesh->stop();
          removeChildComponent (circMesh);
          delete circMesh;
      }
      
      if (twoDVisualizer != nullptr)
      {
          twoDVisualizer->stop();
          removeChildComponent (twoDVisualizer);
          delete twoDVisualizer;
      }
    
    if (squareMesh!= nullptr)
    {
        squareMesh->stop();
        removeChildComponent (squareMesh);
        delete squareMesh;
    }
    
    if (triangleMesh!= nullptr)
    {
        triangleMesh->stop();
        removeChildComponent (triangleMesh);
        delete triangleMesh;
    }
      
      audioSource.releaseResources();
      delete circBuffer;
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

/*
 Overriden function that redraws visuals
 */
void MainComponent::resized()
{
    int width = getWidth();
    int height = getHeight();
    
    //buttons
    int bWidth = (width - 30) / 2;
    int bHeight = 20;
    int bMargin = 10;
    
    resizeButtons(bWidth, bHeight, bMargin);
    
    //Visualizers
    resizeVisualizers(width, height);
}

/*
 Sets up the GUI mainly the text buttons and connects them to the main component
 */
void MainComponent::setupGUI(Listener *mainComponent) {
    //Open file
    addAndMakeVisible(&openFileButton);
    openFileButton.setButtonText("Open File");
    openFileButton.addListener(mainComponent);
    
    //Play Button
    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.addListener(mainComponent);
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.setEnabled(false);
    
    //Stop Button
    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.addListener(mainComponent);
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled(false);
    
    //Visualizer Buttons
    addAndMakeVisible(&twoDButton);
    twoDButton.setButtonText("2D Visualizer");
    twoDButton.setColour(TextButton::textColourOffId, Colours::black);
    twoDButton.setColour(TextButton::buttonColourId, Colours::floralwhite);
    twoDButton.addListener(mainComponent);
    twoDButton.setToggleState(false, NotificationType::dontSendNotification);
    
    addAndMakeVisible(&threeDButton);
    threeDButton.setButtonText("3D Circular Visualizer");
    threeDButton.setColour(TextButton::textColourOffId, Colours::black);
    threeDButton.setColour(TextButton::buttonColourId, Colours::floralwhite);
    threeDButton.addListener(mainComponent);
    threeDButton.setToggleState(false, NotificationType::dontSendNotification);
    
    addAndMakeVisible(&lineVisualizer);
    lineVisualizer.setButtonText("3D Line Visualizer");
    lineVisualizer.setColour(TextButton::textColourOffId, Colours::black);
    lineVisualizer.setColour(TextButton::buttonColourId, Colours::floralwhite);
    lineVisualizer.addListener(mainComponent);
    lineVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    
    addAndMakeVisible(&squareVisualizer);
    squareVisualizer.setButtonText("3D Square Visualizer");
    squareVisualizer.setColour(TextButton::textColourOffId, Colours::black);
    squareVisualizer.setColour(TextButton::buttonColourId, Colours::floralwhite);
    squareVisualizer.addListener(mainComponent);
    squareVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    
    addAndMakeVisible(&triangleVisualizer);
    triangleVisualizer.setButtonText("3D Triangle Visualizer");
    triangleVisualizer.setColour(TextButton::textColourOffId, Colours::black);
    triangleVisualizer.setColour(TextButton::buttonColourId, Colours::floralwhite);
    triangleVisualizer.addListener(mainComponent);
    triangleVisualizer.setToggleState(false, NotificationType::dontSendNotification);

}

/*
 Function that handles resizing buttons based on the specified width, height, and margins of the buttons
 */
void MainComponent::resizeButtons(int bWidth, int bHeight, int bMargins) {
    openFileButton.setBounds(bMargins, bMargins, bWidth, bHeight);
    playButton.setBounds(bMargins, 40, bWidth, bHeight);
    stopButton.setBounds(bMargins, 70, bWidth, bHeight);
    
    twoDButton.setBounds(bWidth + 2 * bMargins, bMargins, bWidth, bHeight);
    threeDButton.setBounds(bWidth + 2 * bMargins, 40, bWidth, bHeight);
    lineVisualizer.setBounds(bWidth+2 * bMargins + 2* bWidth/3 + 2*bMargins/3, 70, bWidth/3, bHeight);
    squareVisualizer.setBounds(bWidth+2 * bMargins , 70, bWidth/3, bHeight);
    triangleVisualizer.setBounds(bWidth+2 * bMargins + bWidth/3 + bMargins/3, 70, bWidth/3, bHeight);

}

/*
 Handles if the audio is playing pausing stopping or stopped
 */
void MainComponent::changeListenerCallback(ChangeBroadcaster *source) {
    if(source == &audioSource) {
        if(audioSource.isPlaying())
            changeAudioState(PLAYING);
        else if(state == STOPPING || state == PLAYING)
            changeAudioState(STOPPED);
        else if(state == PAUSING)
            changeAudioState(PAUSED);
    }
}

/*
 Overriden function that handles all button clicks
 */
void MainComponent::buttonClicked(Button *buttonClicked) {
    if(buttonClicked == &openFileButton) openFile();
    else if(buttonClicked == &playButton) play();
    else if(buttonClicked == &stopButton) stop();
    else if(buttonClicked == &twoDButton) twoDButtonClicked(buttonClicked);
    else if(buttonClicked == &threeDButton) circVisualizerClicked(buttonClicked);
    else if(buttonClicked == &lineVisualizer) lineVisualizerClicked(buttonClicked);
    else if(buttonClicked == &squareVisualizer) squareVisualizerClicked(buttonClicked);
    else if(buttonClicked == &triangleVisualizer) triangleVisualizerClicked(buttonClicked);
}
/*
 Changes the audioState to the new state
 */
void MainComponent::changeAudioState(MainComponent::AudioState newState) {
    if(state != newState) {
        state = newState;
        
        switch (state) {
            case STOPPED:
                playButton.setButtonText("Play");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(false);
                audioSource.setPosition(0.0);
                break;
            case STARTING:
                audioSource.start();
                break;
            case PLAYING:
                playButton.setButtonText("Pause");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(true);
                break;
            case PAUSED:
                playButton.setButtonText("Resume");
                stopButton.setButtonText("Restart");
                break;
            case PAUSING:
                audioSource.stop();
                break;
            case STOPPING:
                audioSource.stop();
                break;
        
        }
    }
}

/*
 Handles file opening takes in any filetype but will only allow execution if it is a playable file
 */
void MainComponent::openFile() {
    FileChooser chooser("File Selection", File(), "", false);
    
    if(chooser.browseForFileToOpen()) {
        File selectedFile (chooser.getResult());
        AudioFormatReader *reader = manager.createReaderFor(selectedFile);
        
        if(reader != nullptr) {
            ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
            audioSource.setSource(newSource, 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            readerSource = newSource.release();
            audioFileEnabled = true;
        }
    }
}
/*
 Function that specifies behavior if the play button is pressed
 */
void MainComponent::play() {
    if(state == STOPPED || state == PAUSED)
        changeAudioState(STARTING);
    else if(state == PLAYING)
        changeAudioState(PAUSING);
}
/*
Function that specifies behavior if the stop button is pressed
*/
void MainComponent::stop() {
    if(state == PAUSED)
        changeAudioState(STOPPED);
    else
        changeAudioState(STOPPING);
}

/*
Function that specifies behavior if the 2D visualizer button is pressed
*/
void MainComponent::twoDButtonClicked(Button *&buttonClicked) {
     bool buttonToggleState = !buttonClicked->getToggleState();
     buttonClicked->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
     triangleVisualizer.setToggleState(false, NotificationType::dontSendNotification);
     threeDButton.setToggleState(false, NotificationType::dontSendNotification);
     squareVisualizer.setToggleState(false, NotificationType::dontSendNotification);
     lineVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    
     twoDVisualizer->setVisible(buttonToggleState);
     squareMesh->setVisible(false);
     lineMesh->setVisible(false);
     circMesh->setVisible(false);
     triangleMesh->setVisible(false);
     
     twoDVisualizer->start();
     triangleMesh->stop();
     circMesh->stop();
     squareMesh->stop();
     lineMesh->stop();
     
     resized();
}

/*
Function that specifies behavior if the 3D Circular Visualizer button is pressed
*/

void MainComponent::circVisualizerClicked(Button *&buttonClicked) {
     bool buttonToggleState = !buttonClicked->getToggleState();
     buttonClicked->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
     twoDButton.setToggleState(false, NotificationType::dontSendNotification);
     triangleVisualizer.setToggleState(false, NotificationType::dontSendNotification);
     squareVisualizer.setToggleState(false, NotificationType::dontSendNotification);
     lineVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    
     circMesh->setVisible(buttonToggleState);
     squareMesh->setVisible(false);
     lineMesh->setVisible(false);
     triangleMesh->setVisible(false);
     twoDVisualizer->setVisible(false);
     
     circMesh->start();
     twoDVisualizer->stop();
     triangleMesh->stop();
     squareMesh->stop();
     lineMesh->stop();
     
     resized();
}

/*
Function that specifies behavior if the 3D Line Visualizer button is pressed
*/
void MainComponent::lineVisualizerClicked(Button *&buttonClicked) {
     bool buttonToggleState = !buttonClicked->getToggleState();
     buttonClicked->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
     twoDButton.setToggleState(false, NotificationType::dontSendNotification);
     threeDButton.setToggleState(false, NotificationType::dontSendNotification);
     squareVisualizer.setToggleState(false, NotificationType::dontSendNotification);
     triangleVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    
     lineMesh->setVisible(buttonToggleState);
     squareMesh->setVisible(false);
     triangleMesh->setVisible(false);
     circMesh->setVisible(false);
     twoDVisualizer->setVisible(false);
     
     lineMesh->start();
     twoDVisualizer->stop();
     circMesh->stop();
     squareMesh->stop();
     triangleMesh->stop();
     
     resized();}

/*
Function that specifies behavior if the 3D Square Visualizer button is pressed
*/

void MainComponent::squareVisualizerClicked(Button *&buttonClicked) {
     bool buttonToggleState = !buttonClicked->getToggleState();
     buttonClicked->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
     twoDButton.setToggleState(false, NotificationType::dontSendNotification);
     threeDButton.setToggleState(false, NotificationType::dontSendNotification);
     triangleVisualizer.setToggleState(false, NotificationType::dontSendNotification);
     lineVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    
     squareMesh->setVisible(buttonToggleState);
     triangleMesh->setVisible(false);
     lineMesh->setVisible(false);
     circMesh->setVisible(false);
     twoDVisualizer->setVisible(false);
     
     squareMesh->start();
     twoDVisualizer->stop();
     circMesh->stop();
     triangleMesh->stop();
     lineMesh->stop();
     
     resized();
}

/*
Function that specifies behavior if the 3D Triangle Visualizer button is pressed
*/
void MainComponent::triangleVisualizerClicked(Button *&buttonClicked) {
    bool buttonToggleState = !buttonClicked->getToggleState();
    buttonClicked->setToggleState(buttonToggleState, NotificationType::dontSendNotification);
    twoDButton.setToggleState(false, NotificationType::dontSendNotification);
    threeDButton.setToggleState(false, NotificationType::dontSendNotification);
    squareVisualizer.setToggleState(false, NotificationType::dontSendNotification);
    lineVisualizer.setToggleState(false, NotificationType::dontSendNotification);
   
    triangleMesh->setVisible(buttonToggleState);
    squareMesh->setVisible(false);
    lineMesh->setVisible(false);
    circMesh->setVisible(false);
    twoDVisualizer->setVisible(false);
    
    triangleMesh->start();
    twoDVisualizer->stop();
    circMesh->stop();
    squareMesh->stop();
    lineMesh->stop();
    
    resized();
}


/*
 Resizes visualizers after checking if the
 */
void MainComponent::resizeVisualizers(int width, int height) {
    if(twoDVisualizer != nullptr)
        twoDVisualizer->setBounds(0, 100, width, height-100);
    if(circMesh != nullptr)
        circMesh->setBounds(0, 100, width, height-100);
    if(lineMesh != nullptr)
        lineMesh->setBounds(0, 100, width, height-100);
    if(triangleMesh != nullptr)
        triangleMesh->setBounds(0, 100, width, height-100);
    if(squareMesh != nullptr)
        squareMesh->setBounds(0, 100, width, height-100);
}


