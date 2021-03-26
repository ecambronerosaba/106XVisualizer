/*
 ==============================================================================
 
 CircularMesh.cpp
 Created: 6 Dec 2019 6:21:03pm
 Author:  Esteban Cambronero
 
 ==============================================================================
 */

#include "CircularMesh.h"
/*
 Constructor for circular mesh takes in a circular buffer and a string as parameters
 */
CircularMesh::CircularMesh(CircularBuffer *buffer, std::string type) : readBuffer(2, CIRC_BUFFER_READ_SIZE), forwardFFT(fftOrder)
{
    meshType = type;
    gLContext.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    circBuffer = buffer;
    
    draggableOrientation.reset(Vector3D<float>(0.0,1.0,0.0));
    
    fftData = new GLfloat[2*fftSize];
    
    gLContext.setRenderer(this);
    gLContext.attachTo(*this);
    
    addAndMakeVisible(statusLabel);
    statusLabel.setJustificationType(Justification::topLeft);
    statusLabel.setFont(Font(14.0f));
}

/*
 Destructor for circular mesh
 */
CircularMesh::~CircularMesh() {
    gLContext.setContinuousRepainting(false);
    gLContext.detach();
    
    delete[] fftData;
    
    circBuffer = nullptr;
}

/*
 Start function that allows for continious repainting which calls the renderOpenGL function
 */
void CircularMesh::start() {
    gLContext.setContinuousRepainting(true);
}

/*
 Stops the mesh from animating
 */
void CircularMesh::stop() {
    gLContext.setContinuousRepainting(false);
}

/*
 Creates new OpenGL context which handles all of the visuals
 */
void CircularMesh::newOpenGLContextCreated() {
    xWidth = 3.0f;
    yHeight = 1.0f;
    zDepth = 3.0f;
    xRes = 80;
    zRes = 81;
    numVertices = xRes * zRes;
    
    initializeGridVertices();
    
    initializeVertVertices();
    
    gLContext.extensions.glGenBuffers (1, &xzVBO); // Vertex Buffer Object
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, xzVBO);
    gLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices * 2, xzVertices, GL_STATIC_DRAW);
    
    
    gLContext.extensions.glGenBuffers (1, &yVBO);
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, yVBO);
    gLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices, yVertices, GL_STREAM_DRAW);
    
    gLContext.extensions.glGenVertexArrays(1, &VAO);
    gLContext.extensions.glBindVertexArray(VAO);
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, xzVBO);
    gLContext.extensions.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, yVBO);
    gLContext.extensions.glVertexAttribPointer (1, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), NULL);
    
    gLContext.extensions.glEnableVertexAttribArray (0);
    gLContext.extensions.glEnableVertexAttribArray (1);
    
    glPointSize (3.0f);
    
    // Setup Shaders
    createShaders();
}

/*
 Deallocates memory when openGL context is closed
 */
void CircularMesh::openGLContextClosing() {
    shader->release();
    shader = nullptr;
    uniforms = nullptr;
    
    delete xzVertices;
    delete yVertices;
}

/*
 Renders the continiously updated graphics from the FFT data
 */
void CircularMesh::renderOpenGL() {
    const float renderingScale = (float) gLContext.getRenderingScale();
    glViewport (0, 0, roundToInt (renderingScale * getWidth()), roundToInt (renderingScale * getHeight()));
    
    // Set background Color
    OpenGLHelpers::clear (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    // Enable Alpha Blending
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader->use();
    
    circBuffer->read (readBuffer, CIRC_BUFFER_READ_SIZE);
    FloatVectorOperations::clear (fftData, CIRC_BUFFER_READ_SIZE);
    
    
    for (int i = 0; i < 2; ++i)
    {
        FloatVectorOperations::add (fftData, readBuffer.getReadPointer(i, 0), CIRC_BUFFER_READ_SIZE);
    }
    
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);
    
    // Find the range of values produced, so we can scale our rendering to
    // show up the detail clearly
    Range<float> maxFFTLevel = FloatVectorOperations::findMinAndMax (fftData, fftSize / 2);
    
    // Calculate new y values and shift old y values back
    for (int i = numVertices; i >= 0; --i)
    {
        // For the first row of points, render the new height via the FFT
        if (i < xRes)
        {
            const float skewedProportionY = 1.0f - std::exp (std::log (i / ((float) xRes - 1.0f)) * 0.2f);
            const int fftDataIndex = jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
            float level = 0.0f;
            
            if (maxFFTLevel.getEnd() != 0.0f)
                level = jmap (fftData[fftDataIndex], 0.0f, maxFFTLevel.getEnd(), 0.0f, yHeight);
            yVertices[i] = level;
        }
        else // For the subsequent rows, shift back
        {
            yVertices[i] = yVertices[i - xRes];
        }
    }
    
    gLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, yVBO);
    gLContext.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof(GLfloat) * numVertices, yVertices, GL_STREAM_DRAW);
    
    
    // Setup the Uniforms for use in the Shader
    if (uniforms->projectionMatrix != nullptr)
        uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);
    
    if (uniforms->viewMatrix != nullptr)
    {
        Matrix3D<float> scale;
        scale.mat[0] = 2.0;
        scale.mat[5] = 2.0;
        scale.mat[10] = 2.0;
        Matrix3D<float> finalMatrix = scale * getViewMatrix();
        uniforms->viewMatrix->setMatrix4 (finalMatrix.mat, 1, false);
        
    }
    
    // Draw the points
    gLContext.extensions.glBindVertexArray(VAO);
    glDrawArrays (GL_POINTS, 0, numVertices);
    
    
    // Zero Out FFT for next use
    zeromem (fftData, sizeof (GLfloat) * 2 * fftSize);
}

/*
 Component function that needs to be overriden but since OpenGL is handling the graphics it is left empty
 */
void CircularMesh::paint(Graphics &g){}

/*
 Resizes the circular mesh as an overriden function from component
 */
void CircularMesh::resized() {
    draggableOrientation.setViewport(getLocalBounds());
    statusLabel.setBounds(getLocalBounds().reduced(4).removeFromTop(75));
}

/*
 allows for setting the position of the mouse down
 */
void CircularMesh::mouseDown(const MouseEvent &e) {
    draggableOrientation.mouseDown(e.getPosition());
}

/*
 Specifies behavior for dragging the mouse
 */
void CircularMesh::mouseDrag(const MouseEvent &e) {
    draggableOrientation.mouseDrag(e.getPosition());
}

/*
 Initializes the xz vertices for the mesh based on the meshtype
 */
void CircularMesh::initializeGridVertices() {
    int numFloatsXZ = numVertices * 2;
    
    xzVertices = new GLfloat [numFloatsXZ];
    
    // Variables when setting x and z
    GLfloat xOffset = xWidth / ((GLfloat) xRes - 1.0f);
    GLfloat zOffset = zDepth / ((GLfloat) zRes - 1.0f);
    GLfloat xStart = -(xWidth / 2.0f);
    GLfloat zStart = -(zDepth / 2.0f);
    int xzIndex = 0;
    if(meshType == "circle") {
        int xCircleOffset = 0;
        for(int col = 0; col < zRes; col++) {
            xCircleOffset = sqrt((zRes/2)*(zRes/2) - (col - zRes/2)*(col - zRes/2));
            for(int row = 0; row < xRes; row++) {
                if(row - zRes/2 < xCircleOffset && row - zRes/2 > -xCircleOffset) {
                    xzVertices[xzIndex] = xStart + xOffset * row;
                    xzVertices[xzIndex + 1] = zStart + zOffset * col;
                }
                else {
                    xzVertices[xzIndex] = -200;
                    xzVertices[xzIndex + 1] = -200;
                }
                xzIndex +=2;
            }
        }
    }
    else if(meshType == "line") {
        for(int col = 0; col < zRes; col++) {
            for(int row = 0; row < xRes; row++) {
                xzVertices[xzIndex] = xStart + xOffset * row;
                xzIndex +=2;
            }
        }
    }
    else if(meshType == "triangle") {
        int topTriangleOffset = 0;
        int bottomTriangleOffset = 0;
        for(int col = 0; col < zRes; col++) {
            bottomTriangleOffset = col/2;
            topTriangleOffset = -col/2 + zRes;
            for(int row = 0; row < xRes; row++) {
                if(row >= bottomTriangleOffset && row <= topTriangleOffset) {
                    xzVertices[xzIndex] = xStart + xOffset * row;
                    xzVertices[xzIndex + 1] = zStart + zOffset * col;
                }
                else {
                    xzVertices[xzIndex] = -200;
                    xzVertices[xzIndex + 1] = -200;
                }
                xzIndex +=2;
            }
        }
    }
    else if(meshType == "square") {
        for(int col = 0; col < zRes; col++) {
            for(int row = 0; row < xRes; row++) {
                xzVertices[xzIndex] = xStart + xOffset * row;
                xzVertices[xzIndex + 1] = zStart + zOffset * col;
                xzIndex +=2;
            }
        }
    }
}

/*
 Initializes vertical vertices for the visualizer
 Made easy because of how the xz vertices are setup
 */
void CircularMesh::initializeVertVertices()
{
    // Set all Y values to 0.0
    yVertices = new GLfloat [numVertices];
    memset(yVertices, 0.0f, sizeof(GLfloat) * xRes * zRes);
}

/*
 Graphics function that helps get the projection matrix of the visualizer
 */
Matrix3D<float> CircularMesh::getProjectionMatrix() const{
    float width = 1.0f / (0.5f + 0.1f);
    float height = width * getLocalBounds().toFloat().getAspectRatio (false);
    return Matrix3D<float>::fromFrustum (-width, width, -height, height, 4.0f, 30.0f);
}

/*
 Graphics function that gets the view Matrix of the visualizer
 */
Matrix3D<float> CircularMesh::getViewMatrix() const {
    Matrix3D<float> viewMatrix((Vector3D<float> (0.0f, 0.0f, -10.0f)));
    Matrix3D<float> rotationMatrix = draggableOrientation.getRotationMatrix();
    
    return rotationMatrix * viewMatrix;
}

/*
 Creates shaders in OpenGL
 */
void CircularMesh::createShaders() {
    VERTEX_SHADER =
    "#version 330 core\n"
    "layout (location = 0) in vec2 xzPos;\n"
    "layout (location = 1) in float yPos;\n"
    // Uniforms
    "uniform mat4 projectionMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = projectionMatrix * viewMatrix * vec4(xzPos[0], yPos, xzPos[1], 1.0f);\n"
    "}\n";
    
    
    // Base Shader
    FRAGMENT_SHADER =
    "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "    color = vec4 (0.0f, 0.749f, 1.0f, 1.0f);\n"
    "}\n";
    
    
    ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (gLContext));
    String statusText;
    
    if (newShader->addVertexShader ((VERTEX_SHADER))
        && newShader->addFragmentShader ((FRAGMENT_SHADER))
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
 Constructor to create the uniforms for the visualizer
 */
CircularMesh::Uniforms::Uniforms(OpenGLContext& context, OpenGLShaderProgram& shaders) {
    projectionMatrix = createUniform(context, shaders, "projectionMatrix");
    viewMatrix = createUniform(context, shaders, "viewMatrix");
}
/*
Creates the uniform based on the name using OpenGL libraries
 */
OpenGLShaderProgram::Uniform* CircularMesh::Uniforms::createUniform(OpenGLContext &context, OpenGLShaderProgram &shaders, const char *uniformName) {
    if(context.extensions.glGetUniformLocation(shaders.getProgramID(), uniformName) < 0)
        return nullptr;
    return new OpenGLShaderProgram::Uniform(shaders, uniformName);
}
