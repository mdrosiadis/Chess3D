// Include C++ headers
#include <iostream>
#include <string>

// Include GLEW (always include first)
#include <GL/glew.h>

// Include GLFW to handle window and keyboard events
#include <glfw3.h>

#include <glm/ext/matrix_transform.hpp>

// Shader loading utilities and other
#include "common/shader.h"
#include "common/util.h"
#include "common/model.h"
#include "common/camera.h"
#include "common/texture.h"

using namespace std;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Chess3D"

// Global variables
GLFWwindow* window;
GLuint chessboardShader, piecesShader;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;
GLuint pieceProjection, pieceView, pieceModel, pieceColor;

void createContext() {
    // Create and compile our GLSL program from the shaders
    chessboardShader = loadShaders("assets/shaders/chessboard.vertex", "assets/shaders/chessboard.fragment");
    piecesShader = loadShaders("assets/shaders/pieces.vertex", "assets/shaders/pieces.fragment");

    projectionMatrixLocation = glGetUniformLocation(chessboardShader, "P");
    viewMatrixLocation = glGetUniformLocation(chessboardShader, "V");
    modelMatrixLocation = glGetUniformLocation(chessboardShader, "M");
    

    pieceProjection = glGetUniformLocation(piecesShader, "P");
    pieceView = glGetUniformLocation(piecesShader, "V");
    pieceModel = glGetUniformLocation(piecesShader, "M");
    pieceColor = glGetUniformLocation(piecesShader, "color");

    // Draw wire frame triangles or fill: GL_LINE, or GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // enable point size
    glEnable(GL_PROGRAM_POINT_SIZE);

    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // enable blending
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // enable textures
    glEnable(GL_TEXTURE_2D);
}

void free() {
    // Free allocated buffers
    glDeleteProgram(chessboardShader);
    glDeleteProgram(piecesShader);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void mainLoop() {
    
    // Create a camera
    Camera camera(window);
    Drawable test("assets/models/blenderfiles/set1/exports/rook.obj");

    GLuint textureSampler = glGetUniformLocation(chessboardShader, "textureSampler");
    GLuint texture = loadSOIL("assets/textures/wood.bmp");

    // create chessboard
    struct BoardPoint 
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uv;
    };
    
    glm::vec3 colors[2] = { 
        { 51.f / 255.f,  36.f / 255.f,  13.f / 255.f},        
        {210.f / 255.f, 169.f / 255.f, 105.f / 255.f}
    };
    
    BoardPoint chessboard_points[9*9];
    GLuint chessboard_elements[2*3*8*8]; 

    // create vertices
    int point_count = 0;
    for(int y=4; y >=-4; --y)
    for(int x=-4; x <=4; ++x)
    {
        chessboard_points[point_count].pos = {(GLfloat) x, 0.0f, (GLfloat) y};
        
        chessboard_points[point_count].color = colors[point_count & 1];
        chessboard_points[point_count].uv = glm::vec2((x+4) / 8.0f, (y+4) / 8.0f);

        point_count++;
    }
    
    // create elements
    int element_count = 0;
    for(int y=0; y < 8; ++y)
    for(int x=0; x < 8; ++x)
    {
        // Triangle 1
        chessboard_elements[element_count++] = (GLuint) y*9+x+1;
        chessboard_elements[element_count++] = (GLuint) (y+1)*9+x+1;
        chessboard_elements[element_count++] = (GLuint) y*9+x;
        
        // Triangle 2
        chessboard_elements[element_count++] = (GLuint) (y+1)*9+x+1;
        chessboard_elements[element_count++] = (GLuint) (y+1)*9+x;
        chessboard_elements[element_count++] = (GLuint) y*9+x;
    }

    // Send data to gpu 
    GLuint chessboardVAO, chessboardVBO, chessboardEBO;
    glGenVertexArrays(1, &chessboardVAO);
    glBindVertexArray(chessboardVAO);

    glGenBuffers(1, &chessboardVBO);
    glBindBuffer(GL_ARRAY_BUFFER, chessboardVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chessboard_points), chessboard_points, GL_STATIC_DRAW);

    glGenBuffers(1, &chessboardEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chessboardEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(chessboard_elements), chessboard_elements, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BoardPoint), NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(BoardPoint), ((GLfloat*) NULL) + 3);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BoardPoint), ((GLfloat*) NULL) + 6);
    glEnableVertexAttribArray(2);

    camera.position = glm::vec3(0.0f, 5.0f, 9.0f);

    do {
        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update();
	
        // Draw
        glUseProgram(chessboardShader);

        // MVP
        glm::mat4 modelMatrix = glm::mat4(1.0);

        // Task 1.4c: transfer uniforms to GPU
        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &camera.projectionMatrix[0][0]);
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &camera.viewMatrix[0][0]);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);

        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        // Set our "textureSampler" sampler to use Texture Unit 0
        glUniform1i(textureSampler, 0);
    
        glBindVertexArray(chessboardVAO);
        glDrawElements(GL_TRIANGLES, 64*6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        modelMatrix = glm::translate(glm::mat4(1.0f), { -3.5f, 0.0f, 3.5f});

        glUseProgram(piecesShader);

        glUniformMatrix4fv(pieceProjection, 1, GL_FALSE, &camera.projectionMatrix[0][0]);
        glUniformMatrix4fv(pieceView, 1, GL_FALSE, &camera.viewMatrix[0][0]);
        glUniformMatrix4fv(pieceModel, 1, GL_FALSE, &modelMatrix[0][0]);
       
        glm::vec3 black(0.0f);
        glUniform3fv(pieceColor, 1, &black[0]);

        test.bind();
        test.draw(); 
		
        // Swap buffers
        glfwSwapBuffers(window);

        // Events
        glfwPollEvents();
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);
}

void initialize() {
    // Initialize GLFW
    if (!glfwInit()) {
        throw runtime_error("Failed to initialize GLFW\n");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);  // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(W_WIDTH, W_HEIGHT, TITLE, NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        throw runtime_error(string(string("Failed to open GLFW window.") +
                            " If you have an Intel GPU, they are not 3.3 compatible." +
                            "Try the 2.1 version.\n"));
    }
    glfwMakeContextCurrent(window);

    // Set Handler function for resizing the window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Start GLEW extension handler
    glewExperimental = GL_TRUE;

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw runtime_error("Failed to initialize GLEW\n");
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set background color (gray) [r, g, b, a]
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);

    // Log
    logGLParameters();
}

int main(void) {
    try {
        initialize();
        createContext();
        mainLoop();
        free();
    } catch (exception& ex) {
        cout << ex.what() << endl;
        getchar();
        free();
        return -1;
    }

    return 0;
}


// glfw: whenever the window size changed this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
