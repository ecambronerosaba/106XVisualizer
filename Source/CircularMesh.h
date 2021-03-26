/*
  ==============================================================================

    CircularMesh.h
    Created: 6 Dec 2019 6:21:03pm
    Author:  Esteban Cambronero

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "CircularBuffer.h"

#define CIRC_BUFFER_READ_SIZE 256

class CircularMesh : public Component, public OpenGLRenderer
{
public:
    CircularMesh(CircularBuffer *circBuffer, std::string type);
    ~CircularMesh();
    void start();
    void stop();
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void renderOpenGL() override;
    void paint(Graphics &g) override;
    void resized() override;
    void mouseDown(const MouseEvent &e) override;
    void mouseDrag(const MouseEvent &e) override;
private:
    void drawGridType();
    void initializeGridVertices();
    void initializeVertVertices();
    Matrix3D<float> getProjectionMatrix() const;
    Matrix3D<float> getViewMatrix() const;
    void createShaders();
    struct Uniforms {
        Uniforms(OpenGLContext &context, OpenGLShaderProgram &shaders);
        ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;
    private:
        static OpenGLShaderProgram::Uniform* createUniform(OpenGLContext &context, OpenGLShaderProgram &shaders, const char *uniformName);
    };
    GLfloat xWidth;
    GLfloat yHeight;
    GLfloat zDepth;
    int xRes;
    int zRes;
    
    int numVertices;
    GLfloat *xzVertices;
    GLfloat *yVertices;
    
    OpenGLContext gLContext;
    GLuint xzVBO;
    GLuint yVBO;
    GLuint VAO;
    
    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<Uniforms> uniforms;
    
    const char* VERTEX_SHADER;
    const char* FRAGMENT_SHADER;
    
    
    // GUI Interaction
    Draggable3DOrientation draggableOrientation;
    
    // Audio Structures
    CircularBuffer * circBuffer;
    AudioBuffer<GLfloat> readBuffer;
    dsp::FFT forwardFFT;
    GLfloat * fftData;
    std::string meshType;

    enum
    {
        fftOrder = 10,
        fftSize  = 1 << fftOrder // set 10th bit to one
    };
    
    Label statusLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CircularMesh)
};
