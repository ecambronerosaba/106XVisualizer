/*
  ==============================================================================

    SineVisualizer.cpp
    Created: 3 Dec 2019 4:44:33pm
    Author:  Esteban Cambronero

  ==============================================================================
*/

#include "SineVisualizer.h"

/*
 Constructor for sine visualizer
 */
SineVisualizer::SineVisualizer(CircularBuffer *cBuffer) : readBuffer(2, CIRC_BUFFER_READ_SIZE){
    gLContext.setOpenGLVersionRequired(OpenGLContext::OpenGLVersion::openGL3_2);
    circBuffer = cBuffer;
    gLContext.setRenderer(this);
    gLContext.attachTo(*this);
    
    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(Justification::topLeft);
    statusLabel.setFont(Font(14.0f));
}

/*
 Destructor for Sine Visualizer
 */
SineVisualizer::~SineVisualizer() {
    gLContext.setContinuousRepainting(false);
    gLContext.detach();
    
    circBuffer = nullptr;
}

/*
 Function that starts the animation of the visualizer
 */
void SineVisualizer::start() {
    gLContext.setContinuousRepainting(true);
}

/*
 Function that stops the animation of the visualizer
 */
void SineVisualizer::stop() {
    gLContext.setContinuousRepainting(false);
}

/*
 Initializes the graphics for OpenGL
 */
void SineVisualizer::newOpenGLContextCreated() {
    createShaders();
    
    gLContext.extensions.glGenBuffers(1, &VBO);
    gLContext.extensions.glGenBuffers(1, &EBO);
}

/*
Deallocates memory when openGL context is closed
*/
void SineVisualizer::openGLContextClosing() {
    shader->release();
    shader = nullptr;
    uniforms = nullptr;
}

/*
Renders the continiously updated graphics from the FFT data
*/
void SineVisualizer::renderOpenGL() {
    float scale = (float) gLContext.getRenderingScale();
    glViewport(0, 0, roundToInt(scale * getWidth()), roundToInt(scale * getHeight()));
    
    OpenGLHelpers::clear(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader->use();
    
    if (uniforms->resolution != nullptr)
            uniforms->resolution->set ((GLfloat) scale * getWidth(), (GLfloat) scale * getHeight());
        
        if (uniforms->sampleData != nullptr)
        {
            circBuffer->read (readBuffer, CIRC_BUFFER_READ_SIZE);
            
            FloatVectorOperations::clear (visualizationBuffer, CIRC_BUFFER_READ_SIZE);
            
            for (int i = 0; i < 2; ++i)
            {
                FloatVectorOperations::add (visualizationBuffer, readBuffer.getReadPointer(i, 0), CIRC_BUFFER_READ_SIZE);
            }
            
            uniforms->sampleData->set (visualizationBuffer, 256);
        }
        
        // Define Vertices for a Square (the view plane)
        GLfloat vertices[] = {
            1.0f,   1.0f,  0.0f,  // Top Right
            1.0f,  -1.0f,  0.0f,  // Bottom Right
            -1.0f, -1.0f,  0.0f,  // Bottom Left
            -1.0f,  1.0f,  0.0f   // Top Left
        };
        // Define Which Vertex Indexes Make the Square
        GLuint indices[] = {
            0, 1, 3,
            1, 2, 3
        };
    
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, VBO);
    gLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

        
    // EBO (Element Buffer Object) - Bind and Write to Buffer
    gLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, EBO);
    gLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW);

        
    // Setup Vertex Attributes
    gLContext.extensions.glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    gLContext.extensions.glEnableVertexAttribArray (0);
    
    // Draw Vertices
    glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // For EBO's (Element Buffer Objects) (Indices)
        
    
        
    // Reset the element buffers so child Components draw correctly
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
    gLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*
 Creates shaders in OpenGL
 */
void SineVisualizer::createShaders() {
        VERTEX_SHADER =
        "attribute vec3 position;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position, 1.0);\n"
        "}\n";
        
        FRAGMENT_SHADER =
        "uniform vec2  resolution;\n"
        "uniform float audioSampleData[256];\n"
        "\n"
        "void getAmplitudeForXPos (in float xPos, out float audioAmplitude)\n"
        "{\n"
        // Buffer size - 1
        "   float perfectSamplePosition = 255.0 * xPos / resolution.x;\n"
        "   int leftSampleIndex = int (floor (perfectSamplePosition));\n"
        "   int rightSampleIndex = int (ceil (perfectSamplePosition));\n"
        "   audioAmplitude = mix (audioSampleData[leftSampleIndex], audioSampleData[rightSampleIndex], fract (perfectSamplePosition));\n"
        "}\n"
        "\n"
        "#define THICKNESS 0.02\n"
        "void main()\n"
        "{\n"
        "    float y = gl_FragCoord.y / resolution.y;\n"
        "    float amplitude = 0.0;\n"
        "    getAmplitudeForXPos (gl_FragCoord.x, amplitude);\n"
        "\n"
        // Centers & Reduces Wave Amplitude
        "    amplitude = 0.5 - amplitude / 2.5;\n"
        "    float r = abs (THICKNESS / (amplitude-y));\n"
        "\n"
        "gl_FragColor = vec4 (r - abs (r * 0.2), r - abs (r * 0.2), r - abs (r * 0.2), 1.0);\n"
        "}\n";
        
        ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (gLContext));
        String statusText;
        
        if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (VERTEX_SHADER))
            && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (FRAGMENT_SHADER))
            && newShader->link())
        {
            uniforms = nullptr;
            
            shader = newShader;
            shader->use();
            
            uniforms   = new Uniforms (gLContext, *shader);
            
            statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
        }
        else
        {
            statusText = newShader->getLastError();
        }
}

/*
Creates the uniform based on the name using OpenGL libraries
 */
OpenGLShaderProgram::Uniform* SineVisualizer::Uniforms::createUniform(OpenGLContext &openGL, OpenGLShaderProgram &shader, const char *uniformName) {
    if(openGL.extensions.glGetUniformLocation(shader.getProgramID(), uniformName) < 0)
        return nullptr;
    return new OpenGLShaderProgram::Uniform (shader, uniformName);
}
/*
Resizes the circular mesh as an overriden function from component
*/
void SineVisualizer::resized() {
    statusLabel.setBounds(getLocalBounds().reduced(4).removeFromTop(75));
}
/*
Component function that needs to be overriden but since OpenGL is handling the graphics it is left empty
*/
void SineVisualizer::paint(Graphics &g) {};

/*
Constructor to create the uniforms for the visualizer
*/
SineVisualizer::Uniforms::Uniforms(OpenGLContext &openContext, OpenGLShaderProgram &shader) {
    resolution = createUniform(openContext, shader, "resolution");
    sampleData = createUniform(openContext, shader, "audioSampleData");
}
