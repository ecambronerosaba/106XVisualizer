/*
  ==============================================================================

    SineVisualizer.h
    Created: 3 Dec 2019 4:44:33pm
    Author:  Esteban Cambronero

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "CircularBuffer.h"

#define CIRC_BUFFER_READ_SIZE 256

class SineVisualizer : public Component, public OpenGLRenderer

{
public:
    SineVisualizer(CircularBuffer *circBuffer);
    ~SineVisualizer();
    
    void start();
    void stop();
    
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void renderOpenGL() override;
    void paint(Graphics &g) override;
    void resized() override;
private:
    void createShaders();
    struct Uniforms {
        Uniforms(OpenGLContext &openGLContext, OpenGLShaderProgram &shaderProgram);
        ScopedPointer<OpenGLShaderProgram::Uniform> resolution, sampleData;
    private:
    static OpenGLShaderProgram::Uniform *createUniform (OpenGLContext &openGL, OpenGLShaderProgram &shader, const char *uniformName);
    };
    OpenGLContext gLContext;
    GLuint VBO, VAO, EBO;
    
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<Uniforms> uniforms;
    
    const char *VERTEX_SHADER;
    const char *FRAGMENT_SHADER;
    
    CircularBuffer *circBuffer;
    AudioBuffer<GLfloat> readBuffer;
    Label statusLabel;
    GLfloat visualizationBuffer[CIRC_BUFFER_READ_SIZE];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SineVisualizer)
};
