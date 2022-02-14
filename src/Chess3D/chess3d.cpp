// Include C++ headers
#include <iostream>
#include <string>

// Include GLEW (always include first)
#include <GL/glew.h>

// Include GLFW to handle window and keyboard events
#include <glfw3.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// Shader loading utilities and other
#include "common/shader.h"
#include "common/util.h"
#include "common/model.h"
#include "common/camera.h"
#include "common/texture.h"
#include "common/light.h"

using namespace std;

// Function prototypes
void initialize();
void createContext();
void mainLoop();
void free();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void lighting_pass();
void depth_pass(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

#define W_WIDTH 1024
#define W_HEIGHT 768
#define TITLE "Chess3D"

#define SHADOW_WIDTH 4096
#define SHADOW_HEIGHT 4096

struct Material {
    glm::vec4 Ka;
    glm::vec4 Kd;
    glm::vec4 Ks;
	float Ns;
};

const Material polishedSilver{
	{  0.0215,    0.1475,   0.0215, 0.55},
	{ 0.07568,   0.61424,  0.07568, 0.55},
	{   0.633,  0.727811,    0.633, 0.55},
	76.8f
};

static Light sceneLight;
static Camera camera;
static Drawable *rook;

int cursorFile = 0, cursorRank = 0;
// Global variables
GLFWwindow* window;
GLuint chessboardShader, piecesShader, shadowShader, depthShader;
GLuint projectionMatrixLocation, viewMatrixLocation, modelMatrixLocation;
GLuint pieceProjection, pieceView, pieceModel, pieceColor;
GLuint chessboardVAO, chessboardVBO, chessboardEBO;
GLuint textureSampler, texture;
GLuint KaLocation, KdLocation, KsLocation, NsLocation;
GLuint LaLocation, LdLocation, LsLocation;
GLuint lightPositionLocation;
GLuint lightPowerLocation;
GLuint diffuseColorSampler; 
GLuint depthMapSampler;
GLuint lightVPLocation;
GLuint lightDirectionLocation;
GLuint lightFarPlaneLocation;
GLuint lightNearPlaneLocation;
GLuint shadowViewProjectionLocation, shadowModelLocation;
GLuint depthFrameBuffer, depthTexture;
GLuint colorsLocation;
GLuint selectionsLocation;

int selections[8][8] = {0};

// Global chessboard colors
glm::vec3 colors[2] = { 
    { 51.f / 255.f,  36.f / 255.f,  13.f / 255.f},        
    {210.f / 255.f, 169.f / 255.f, 105.f / 255.f}
};

void create_chessboard();

void uploadLightToShader(GLuint shader, const Light& light) {
	glUniform4f(glGetUniformLocation(shader, "light.La"), light.La.r, light.La.g, light.La.b, light.La.a);
	glUniform4f(glGetUniformLocation(shader, "light.Ld"), light.Ld.r, light.Ld.g, light.Ld.b, light.Ld.a);
	glUniform4f(glGetUniformLocation(shader, "light.Ls"), light.Ls.r, light.Ls.g, light.Ls.b, light.Ls.a);
	glUniform3f(glGetUniformLocation(shader, "light.lightPosition_worldspace"), light.lightPosition_worldspace.x,
		light.lightPosition_worldspace.y, light.lightPosition_worldspace.z);
	glUniform1f(glGetUniformLocation(shader, "light.power"), light.power);
    
	const glm::mat4 lightVp = sceneLight.projectionMatrix * sceneLight.viewMatrix;
	glUniformMatrix4fv(glGetUniformLocation(shader, "lightVP"), 1, GL_FALSE, &lightVp[0][0]);
}


// Creating a function to upload the material parameters of a model to the shader program
void uploadMaterial(const Material& mtl) {
	glUniform4f(KaLocation, mtl.Ka.r, mtl.Ka.g, mtl.Ka.b, mtl.Ka.a);
	glUniform4f(KdLocation, mtl.Kd.r, mtl.Kd.g, mtl.Kd.b, mtl.Kd.a);
	glUniform4f(KsLocation, mtl.Ks.r, mtl.Ks.g, mtl.Ks.b, mtl.Ks.a);
	glUniform1f(NsLocation, mtl.Ns);
}
void renderScene(bool renderDepth=false)
{

    glm::mat4 modelMatrix = glm::mat4(1.0);

    // Task 1.4c: transfer uniforms to GPU

    if(!renderDepth)
    {
        glUseProgram(chessboardShader);
        
        // uploading the light parameters to the shader program
        uploadLightToShader(chessboardShader, sceneLight);
        glUniform3fv(colorsLocation, 2, &colors[0][0]);

        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &camera.projectionMatrix[0][0]);
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &camera.viewMatrix[0][0]);
        glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &modelMatrix[0][0]);
        
        glUniform1iv(selectionsLocation, 64, &selections[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glUniform1i(glGetUniformLocation(chessboardShader, "shadowMapSampler") ,0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUniform1i(textureSampler, 1);
    }
    else
    {
        glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    }

    glBindVertexArray(chessboardVAO);
    glDrawElements(GL_TRIANGLES, 64*6, GL_UNSIGNED_INT, 0);

    /* double tn = glfwGetTime(); */
    /* float offset = (tn / 3.0) - ((int) (tn / 3.0)); */

    modelMatrix = glm::translate(glm::mat4(1.0f), { -3.5f + cursorFile, 0.0f, 3.5f - cursorRank});

    if(!renderDepth)
    {
        glUseProgram(shadowShader);

        glUniformMatrix4fv(pieceProjection, 1, GL_FALSE, &camera.projectionMatrix[0][0]);
        glUniformMatrix4fv(pieceView, 1, GL_FALSE, &camera.viewMatrix[0][0]);
        glUniformMatrix4fv(pieceModel, 1, GL_FALSE, &modelMatrix[0][0]);

        uploadLightToShader(shadowShader, sceneLight);
        uploadMaterial(polishedSilver);
    }
    else
    {
        glUniformMatrix4fv(shadowModelLocation, 1, GL_FALSE, &modelMatrix[0][0]);
    }
   
    rook->bind();
    rook->draw();
}

void createContext() {
    // Create and compile our GLSL program from the shaders
    chessboardShader = loadShaders("assets/shaders/chessboard.vertex", "assets/shaders/chessboard.fragment");
    /* piecesShader = loadShaders("assets/shaders/pieces.vertex", "assets/shaders/pieces.fragment"); */
    
    shadowShader = loadShaders("assets/shaders/shadowmapping.vetrex", "assets/shaders/shadowmapping.fragment");
    depthShader = loadShaders("assets/shaders/depth.vertex", "assets/shaders/depth.fragment");

    
    projectionMatrixLocation = glGetUniformLocation(chessboardShader, "P");
    viewMatrixLocation = glGetUniformLocation(chessboardShader, "V");
    modelMatrixLocation = glGetUniformLocation(chessboardShader, "M");
    selectionsLocation = glGetUniformLocation(chessboardShader, "selections"); 

    pieceProjection = glGetUniformLocation(shadowShader, "P");
    pieceView = glGetUniformLocation(shadowShader, "V");
    pieceModel = glGetUniformLocation(shadowShader, "M");
    // pieceColor = glGetUniformLocation(piecesShader, "color");

    colorsLocation = glGetUniformLocation(chessboardShader, "colors");

    // Draw wire frame triangles or fill: GL_LINE, or GL_FILL
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // enable point size
    glEnable(GL_PROGRAM_POINT_SIZE);

    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // enable blending
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // enable textures
    glEnable(GL_TEXTURE_2D);

    
	// Tell opengl to generate a framebuffer
	glGenFramebuffers(1, &depthFrameBuffer);
	// Binding the framebuffer, all changes bellow will affect the binded framebuffer
	// **Don't forget to bind the default framebuffer at the end of initialization
	glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);


	// We need a texture to store the depth image
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	// Telling opengl the required information about the texture
	// Use the depth component to create the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);							// Task 4.5 Texture wrapping methods
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);							// GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
	

	// Task 4.5 Don't shadow area out of light's viewport
	// Step 1 : (Don't forget to comment out the respective lines above
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Set color to set out of border 
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// Next go to fragment shader and add an if statement, so if the distance in the z-buffer is equal to 1, 
	// meaning that the fragment is out of the texture border (or further than the far clip plane) 
	// then the shadow value is 0.
	 
	// Task 3.2 Continue
	// Attaching the texture to the framebuffer, so that it will monitor the depth component
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	

	// Since the depth buffer is only for the generation of the depth texture, 
	// there is no need to have a color output
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);


	// Finally, we have to always check that our frame buffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		glfwTerminate();
		throw runtime_error("Frame buffer not initialized correctly");
	}

	// Binding the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
	// for phong lighting
	KaLocation = glGetUniformLocation(shadowShader, "mtl.Ka");
	KdLocation = glGetUniformLocation(shadowShader, "mtl.Kd");
	KsLocation = glGetUniformLocation(shadowShader, "mtl.Ks");
	NsLocation = glGetUniformLocation(shadowShader, "mtl.Ns");
	
	// locations for shadow rendering
	depthMapSampler = glGetUniformLocation(shadowShader, "shadowMapSampler");
	lightVPLocation = glGetUniformLocation(shadowShader, "lightVP");


	// --- depthProgram ---
	shadowViewProjectionLocation = glGetUniformLocation(depthShader, "VP");
	shadowModelLocation = glGetUniformLocation(depthShader, "M");

    create_chessboard();

    camera = Camera(window);
	camera.position = glm::vec3(0.f, 5.f, 9.f);
	camera.lookTo = glm::vec3(0.f, 0.f, 0.f);

    rook = new Drawable("assets/models/rook.obj");

    sceneLight = Light(window, {1.0f, 1.0f, 1.0f, 1.0f},
                               {1.0f, 1.0f, 1.0f, 1.0f},
                               {1.0f, 1.0f, 1.0f, 1.0f},
                               {0.0f, 7.0f, 0.0f},
                               200.f);

    /* selections[1][2] = 1; */
}

void free() {
    // Free allocated buffers
    glDeleteProgram(chessboardShader);
    /* glDeleteProgram(piecesShader); */
    glDeleteProgram(shadowShader);
    glDeleteProgram(depthShader);
    delete rook;

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

void create_chessboard()
{
    // create chessboard
    struct BoardPoint 
    {
        glm::vec3 pos;
        int n;
        glm::vec2 uv;
    };
    
    
    BoardPoint chessboard_points[9*9];
    GLuint chessboard_elements[2*3*8*8]; 

    // create vertices
    int point_count = 0;
    int sqcount = 0;
    for(int y=4; y >=-4; --y)
    for(int x=-4; x <=4; ++x)
    {
        chessboard_points[point_count].pos = {(GLfloat) x, 0.0f, (GLfloat) y};
        
        chessboard_points[point_count].n = sqcount;
        chessboard_points[point_count].uv = glm::vec2((x+4) / 8.0f, (y+4) / 8.0f);
        
        if(x != 4) sqcount++;

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

    glVertexAttribIPointer(1, 1, GL_INT, sizeof(BoardPoint), ((GLfloat*) NULL) + 3);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(BoardPoint), ((GLfloat*) NULL) + 4);
    glEnableVertexAttribArray(2);

}

void mainLoop() {
    
    textureSampler = glGetUniformLocation(chessboardShader, "diffuseColorSampler");
    texture = loadSOIL("assets/textures/wood.bmp");

	int arrow_state[4] = {GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE, GLFW_RELEASE};
	int ARROW_KEYS[4] = {GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_RIGHT, GLFW_KEY_LEFT};

    do {
        // Clear the screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Clear selections
		for(int f=0; f < 8; f++)
		for(int r=0; r < 8; r++)
			selections[f][r] = 0;

        // Events
        glfwPollEvents();
		for(int i=0; i < 4; i++)
		{
			switch(glfwGetKey(window, ARROW_KEYS[i]))
			{
				case GLFW_PRESS:
					if(arrow_state[i] == GLFW_RELEASE)
					{
						int delta = i % 2 ? -1 : 1;
						if(i < 2) cursorRank += delta;
						else	  cursorFile += delta;
					}
					arrow_state[i] = GLFW_PRESS;
					break;
				case GLFW_RELEASE:
					arrow_state[i] = GLFW_RELEASE;
					break;
			}
		}
		cursorRank = std::max(std::min(cursorRank, 7), 0);
		cursorFile = std::max(std::min(cursorFile, 7), 0);

		selections[cursorRank][cursorFile] = 1;

        camera.update();
        sceneLight.update();

		
        // Draw
        depth_pass(sceneLight.viewMatrix, sceneLight.projectionMatrix);
        lighting_pass();

        // Swap buffers
        glfwSwapBuffers(window);

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



void depth_pass(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {

	// Setting viewport to shadow map size
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	// Binding the depth framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthFrameBuffer);

	// Cleaning the framebuffer depth information (stored from the last render)
	glClear(GL_DEPTH_BUFFER_BIT);

	// Selecting the new shader program that will output the depth component
	glUseProgram(depthShader);

	// sending the view and projection matrix to the shader
	const glm::mat4 view_projection = projectionMatrix * viewMatrix ;
	glUniformMatrix4fv(shadowViewProjectionLocation, 1, GL_FALSE, &view_projection[0][0]);


	// ---- rendering the scene ---- //
    renderScene(true);
	// binding the default framebuffer again
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void lighting_pass() {

	// Step 1: Binding a frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, W_WIDTH, W_HEIGHT);

	// Step 2: Clearing color and depth info
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene();
}
