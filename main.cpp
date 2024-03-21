#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 lightPoz;
glm::vec3 lightPoz1;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPozLoc;
GLint lightPozLoc1;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D scena;
gps::Model3D rata;

glm::vec3 rata_poz(-18.8407f, -0.478397f, -20.8434f);
glm::vec3 rata_punct(-18.4977f, -0.478397f, -17.1466f);
float delta = 0.0f;
float last_frame = 0.0f;
float unghi_rata = 0.0f;
float viteza_rata = 20.0f;
float luminaDir = 1;
float rotateCamera = 0;

GLfloat angle;

// shaders
gps::Shader myBasicShader;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLfloat fogDensity;
GLuint fogDensityLoc;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (pressedKeys[GLFW_KEY_P]) {
        if (luminaDir == 1) {
            luminaDir = 0;
        }
        else {
            luminaDir = 1;
        }
        myBasicShader.useShaderProgram();
        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "luminaDir"), luminaDir);
    }

    if (pressedKeys[GLFW_KEY_I]) {
        if (rotateCamera == 0) {
            rotateCamera = 1;
        }
        else {
            rotateCamera = 0;
        }
    }
}

float lastX = 400, lastY = 300;

bool firstMouse = 1;
float yaw = -90.0f;
float pitch = 0.0f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 80.0f)
        fov = 80.0f;
    myBasicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(fov), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for teapot
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
    }
    //solid
    if (pressedKeys[GLFW_KEY_M]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    //wireframe
    if (pressedKeys[GLFW_KEY_N]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //poligonal
    if (pressedKeys[GLFW_KEY_O]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    //smooth
    if (pressedKeys[GLFW_KEY_L]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_SMOOTH);
    }

    if (pressedKeys[GLFW_KEY_F]) {
        fogDensity += 0.002f;
        if (fogDensity >= 0.3f)
            fogDensity = 0.3f;
        myBasicShader.useShaderProgram();
        glUniform1fv(fogDensityLoc, 1, &fogDensity);
    }

    if (pressedKeys[GLFW_KEY_G]) {
        fogDensity -= 0.002f;
        if (fogDensity <= 0.0f)
            fogDensity = 0.0f;
        myBasicShader.useShaderProgram();
        glUniform1fv(fogDensityLoc, 1, &fogDensity);
    }

}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scroll_callback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    //teapot.LoadModel("models/teapot/teapot20segUT.obj");
    scena.LoadModel("models/proiect/Proiect_PG.obj");
    rata.LoadModel("models/proiect/Rata.obj");
}

void initShaders() {
	myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(fov),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPoz = glm::vec3(0.0f, 3.10962f, 32.5094f);
    lightPozLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos");
    glUniform3fv(lightPozLoc, 1, glm::value_ptr(lightPoz));

    lightPoz1 = glm::vec3(29.6457f, 3.3108f, -16.5229f);
    lightPozLoc1 = glGetUniformLocation(myBasicShader.shaderProgram, "lightPos1");
    glUniform3fv(lightPozLoc1, 1, glm::value_ptr(lightPoz1));

    fogDensity = 0.0f;
    fogDensityLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1fv(fogDensityLoc, 1, &fogDensity);
}


void rata_animatie() {
    unghi_rata += viteza_rata * delta;
}

void renderScena(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    scena.Draw(shader);
}

void desenare_rata(gps::Shader shader) {
    shader.useShaderProgram();
    glm::mat4 translatie = glm::translate(glm::mat4(1.0f), rata_punct);
    glm::mat4 rotatie = glm::rotate(glm::mat4(1.0f), glm::radians(unghi_rata), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 translatie2 = glm::translate(glm::mat4(1.0f), -rata_punct);
    glm::mat4 matrice = model * translatie * rotatie * translatie2;
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(matrice));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    rata.Draw(shader);
}

void renderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render the scene
    
	// render the teapot
	renderScena(myBasicShader);
    desenare_rata(myBasicShader);
    rata_animatie();
    mySkyBox.Draw(skyboxShader, view, projection);


}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

void initSkybox() {
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/right.jpg");
    faces.push_back("skybox/left.jpg");
    faces.push_back("skybox/top.jpg");
    faces.push_back("skybox/bottom.jpg");
    faces.push_back("skybox/back.jpg");
    faces.push_back("skybox/front.jpg");
    mySkyBox.Load(faces);
}



int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initSkybox();
    setWindowCallbacks();


	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {

        float currentFrame = glfwGetTime();
        delta = currentFrame - last_frame;
        last_frame = currentFrame;

        if (rotateCamera) {
            float cameraY = 40.0f;
            float radius = 40.0f; 
            float cameraX = sin(glfwGetTime()) * radius;
            float cameraZ = cos(glfwGetTime()) * radius;

            glm::vec3 cameraPosition = glm::vec3(cameraX, cameraY, cameraZ);
            glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

            view = glm::lookAt(cameraPosition, cameraTarget, cameraUp);
        }

        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
