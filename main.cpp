//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#define GLM_SWIZZLE
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"

#include <iostream>

#include "SkyBox.hpp"
using std::vector;

glm::vec3 boundingBoxObj1MIN = glm::vec3(0.0f) ;// first row is for mins, second for max
glm::vec3 boundingBoxObj1MAX = glm::vec3(0.0f) ;

glm::vec3 boundingBoxObj2MIN = glm::vec3(0.0f);// first row is for mins, second for max
glm::vec3 boundingBoxObj2MAX = glm::vec3(0.0f);

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

GLuint textureID;
std::vector<const GLchar*> faces;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;
glm::mat4 modelET;

int fog = 0;
int colorReflection = 0;
GLuint fogLoc, colorReflectionLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 2.0f, 5.5f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
float angleY = 0.0f;
float angleRotate = 0.0f;
GLfloat lightAngle;


gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Model3D myModel;
gps::Model3D cactus;
glm::mat4 modelCactus;
gps::Model3D cactus2;
glm::mat4 modelCactus2;
gps::Model3D ufo;
glm::mat4 modelUfo;
gps::Model3D flag;
glm::mat4 modelFlag;
glm::mat4 modelGround;
glm::mat4 modelTopUfo;

glm::mat4 modelLightMatrix;
gps::Model3D modelLight;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader simpleDepthMap;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = retina_width / 2.0;
float lastY = retina_height / 2.0;
bool pressLineSmooth = false;
bool pressPolySmooth = false;
bool pressAnimation = false;
bool pressBackword = false;
bool rotateUfo = false;
bool panorama = false;
bool goDown = false;

float delta = 0;
bool print = false;
float dx, dz = 0;
float dist = 0;

void computeBoundingBox(gps::Model3D &obj, glm::vec3 &boundingBoxMIN, glm::vec3 &boundingBoxMAX) {
	
	for (gps::Mesh mesh : obj.meshes)
	{
		for (gps::Vertex v : mesh.vertices)
		{
			if ( v.Position.x < boundingBoxMIN.x)
				boundingBoxMIN.x = v.Position.x;

			if (v.Position.y < boundingBoxMIN.y)
				boundingBoxMIN.y = v.Position.y;

			if (v.Position.z < boundingBoxMIN.z)
				boundingBoxMIN.z = v.Position.z;

			if (v.Position.x > boundingBoxMAX.x)
				boundingBoxMAX.x = v.Position.x;

			if (v.Position.y > boundingBoxMAX.y)
				boundingBoxMAX.y = v.Position.y;

			if (v.Position.z > boundingBoxMAX.z)
				boundingBoxMAX.z = v.Position.z;
		}
	}
}

float movementSpeed = 2; // units per second
void updateDelta(double elapsedSeconds)
{
	delta = delta + movementSpeed * elapsedSeconds;
}
double lastTimeStamp = glfwGetTime();

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.15f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[ GLFW_KEY_B])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_V])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_Z])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_N])
	{
		if (!pressPolySmooth) {
			glEnable(GL_POLYGON_SMOOTH);
			pressPolySmooth = !pressPolySmooth;
		}		
		else {
			glDisable(GL_POLYGON_SMOOTH);
			pressPolySmooth = !pressPolySmooth;
		}
	}

	if (pressedKeys[GLFW_KEY_C])
	{
		if (!pressPolySmooth) {
			glEnable(GL_LINE_SMOOTH);
			pressLineSmooth = !pressLineSmooth;
		}
		else {
			glDisable(GL_LINE_SMOOTH);
			pressLineSmooth = !pressLineSmooth;
		}
	}

	if (pressedKeys[GLFW_KEY_UP])
	{
		pressAnimation = !pressAnimation;
	}

	if (pressedKeys[GLFW_KEY_F]) // fog
	{
		myCustomShader.useShaderProgram();
		fog = 1-fog;
		fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fog");
		glUniform1i(fogLoc, fog);
		skyboxShader.useShaderProgram();
		fogLoc = glGetUniformLocation(skyboxShader.shaderProgram, "fog");
		glUniform1i(fogLoc, fog);
	}
	
	if (pressedKeys[GLFW_KEY_R]) // colorReflection
	{
		myCustomShader.useShaderProgram();
		colorReflection = 1-colorReflection;
		colorReflectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "colorReflection");
		glUniform1i(colorReflectionLoc, colorReflection);
	}

	if (pressedKeys[GLFW_KEY_RIGHT])
	{
		if (angleRotate >= -20.0f)
		angleRotate -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_LEFT])
	{
		if(angleRotate<=20.0f)
		angleRotate += 1.0f;
	}
	
	if (pressedKeys[GLFW_KEY_4])
	{		
		//if (boundingBoxObj2MIN.x != boundingBoxObj1MAX.x && boundingBoxObj2MAX.x != boundingBoxObj1MIN.x) {
		//if (dx >= -200)
			dx -= 0.5;
			/*boundingBoxObj1MAX.x -= 0.001;
			boundingBoxObj1MIN.x -= 0.001;
			std::cout << boundingBoxObj1MIN.x << " " << boundingBoxObj1MIN.y << " " << boundingBoxObj1MIN.z << " " << std::endl;
			std::cout << boundingBoxObj1MAX.x << " " << boundingBoxObj1MAX.y << " " << boundingBoxObj1MAX.z << " " << std::endl;*/
		//}
			
	}

	if (pressedKeys[GLFW_KEY_6])
	{
		
		//if (boundingBoxObj2MIN.x != boundingBoxObj1MAX.x && boundingBoxObj2MAX.x != boundingBoxObj1MIN.x){
		//if (dx <= 200)
			dx += 0.5;
			/*boundingBoxObj1MAX.x += 0.001;
			boundingBoxObj1MIN.x += 0.001;
			std::cout << boundingBoxObj1MIN.x << " " << boundingBoxObj1MIN.y << " " << boundingBoxObj1MIN.z << " " << std::endl;
			std::cout << boundingBoxObj1MAX.x << " " << boundingBoxObj1MAX.y << " " << boundingBoxObj1MAX.z << " " << std::endl;*/
		//}
			
	}

	if (pressedKeys[GLFW_KEY_2])
	{		
		//if (boundingBoxObj1MIN.y != boundingBoxObj1MAX.y && boundingBoxObj2MAX.y != boundingBoxObj1MIN.y) {
			dz -= 0.5;
			/*boundingBoxObj1MAX.z += 0.01;
			boundingBoxObj1MIN.z += 0.01;
			std::cout << boundingBoxObj1MIN.x << " " << boundingBoxObj1MIN.y << " " << boundingBoxObj1MIN.z << " " << std::endl;
			std::cout << boundingBoxObj1MAX.x << " " << boundingBoxObj1MAX.y << " " << boundingBoxObj1MAX.z << " " << std::endl;
			std::cout << std::endl;
			    for (int i = 0; i < 4; i++) {
				     for (int j = 0; j < 4; j++) {
					     std::cout << " " << modelET[i][j];					
				}
				std::cout << std::endl;
				
			}*/

	}

	if (pressedKeys[GLFW_KEY_8])
	{
		//if (boundingBoxObj2MIN.y != boundingBoxObj1MAX.y && boundingBoxObj2MAX.y != boundingBoxObj1MIN.y){
		//if (dz <= 200)
			dz += 0.5;
			/*boundingBoxObj1MAX.z += 0.01;
			boundingBoxObj1MIN.z += 0.01;
			std::cout << boundingBoxObj1MIN.x << " " << boundingBoxObj1MIN.y << " " << boundingBoxObj1MIN.z << " " << std::endl;
			std::cout << boundingBoxObj1MAX.x << " " << boundingBoxObj1MAX.y << " " << boundingBoxObj1MAX.z << " " << std::endl;*/
		//}
	}

	if (pressedKeys[GLFW_KEY_P]) {
		panorama = !panorama;
		
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	
	ground.LoadModel("objects/soil/soil.obj", "textures/");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	myModel.LoadModel("objects/alien/My_Sculpture0.obj", "objects/alien/");
	cactus.LoadModel("objects/cactus1/10436_Cactus_v1_max2010_it2.obj", "objects/cactus1/");
	cactus2.LoadModel("objects/cactus1/10436_Cactus_v1_max2010_it2.obj", "objects/cactus1/");
	ufo.LoadModel("objects/ufo/Low_poly_UFO.obj", "objects/ufo/");
	modelLight.LoadModel("objects/cube/cube.obj");
	//flag.LoadModel("objects/flag/flag_obj.obj", "objects/flag/");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	simpleDepthMap.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
	simpleDepthMap.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	//lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	
	lightDir = glm::vec3(0.0f, 5.0f, 1.0f);// by me
	//lightDir = glm::vec3(0.0f, 0.5f, 1.0f);just for cactus
	
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 1.0f, far_plane = 150.0f;
	glm::mat4 lightProjection = glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader) {
		
	shader.useShaderProgram();

	gps::Mesh bottomUfo = ufo.meshes[0];
	modelUfo = glm::mat4(1.0f);
	modelUfo = glm::scale(modelUfo, glm::vec3(0.05f, 0.05f, 0.05f));
	modelUfo = glm::translate(modelUfo, glm::vec3(-60, 10, 20));
	modelUfo = glm::rotate(modelUfo, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	modelUfo = glm::rotate(modelUfo, glm::radians(180.0f), glm::vec3(1, 0, 0));

	//----------------------------------------------top Ufo------------------------------------------------
	gps::Mesh topUfo = ufo.meshes[1];
	modelTopUfo = glm::mat4(1.0f);
	modelTopUfo = glm::scale(modelTopUfo, glm::vec3(0.05f, 0.05f, 0.05f));
	modelTopUfo = glm::translate(modelTopUfo, glm::vec3(-60, 10, 20));
	modelTopUfo = glm::rotate(modelTopUfo, glm::radians(-180.0f), glm::vec3(0, 0, 1));
	modelTopUfo = glm::rotate(modelTopUfo, glm::radians(180.0f), glm::vec3(1, 0, 0));
	

	//animation
	if (pressAnimation) {
		double currentTimeStamp = glfwGetTime();
		float oldDelta = delta;
		updateDelta(currentTimeStamp - lastTimeStamp);
		lastTimeStamp = currentTimeStamp;
		modelUfo = glm::translate(modelUfo, glm::vec3( 0, delta / 5, 0));		
		modelUfo = glm::rotate(modelUfo, 3.14f, glm::vec3(0, 1, 0));

		modelTopUfo = glm::translate(modelTopUfo, glm::vec3(0, delta / 5, 0));		
		modelTopUfo = glm::rotate(modelTopUfo, 3.14f, glm::vec3(0, 1, 0));
		std::cout << oldDelta << " " << delta << std::endl;
	}
	else {
		modelUfo = glm::translate(modelUfo, glm::vec3(0, delta / 5, 0));		
		modelUfo = glm::rotate(modelUfo, 3.14f, glm::vec3(0, 1, 0));

		modelTopUfo = glm::translate(modelTopUfo, glm::vec3(0, delta / 5, 0));		
		modelTopUfo = glm::rotate(modelTopUfo, 3.14f, glm::vec3(0, 1, 0));
	}

	//------perform rotation
	modelTopUfo = glm::rotate(modelTopUfo, glm::radians(angleRotate), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelTopUfo));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * modelTopUfo));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	topUfo.Draw(shader);
	//------------------------------------------------------------------------------------------------------

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelUfo));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * modelUfo));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//ufo.Draw(shader);
	bottomUfo.Draw(shader);
	
	//------------------------------------------------ET-------------------------------------------------------
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	
	model = glm::rotate(model, angleY, glm::vec3(0, 0, 1));
	model = glm::translate(model, glm::vec3(dx, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, dz, 0.0f));
	modelET = model;

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	myModel.Draw(shader);

	//--------------------------------------------------------------------------------------------------------

	modelCactus = glm::mat4(1.0f);
	modelCactus = glm::scale(modelCactus, glm::vec3(0.009f, 0.009f, 0.009f));
	modelCactus = glm::rotate(modelCactus, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	modelCactus = glm::translate(modelCactus, glm::vec3(-100, 600, -150));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelCactus));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * modelCactus));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	cactus.Draw(shader);

	modelCactus2 = glm::mat4(1.0f);
	modelCactus2 = glm::scale(modelCactus2, glm::vec3(0.006f, 0.006f, 0.006f));
	modelCactus2 = glm::rotate(modelCactus2, glm::radians(-90.0f), glm::vec3(1, 0, 0));
	modelCactus2 = glm::rotate(modelCactus2, glm::radians(180.0f), glm::vec3(0, 0, 1));
	modelCactus2 = glm::translate(modelCactus2, glm::vec3(-700, -480, -205));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelCactus2));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * modelCactus2));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	cactus2.Draw(shader);

	modelGround = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
	modelGround = glm::scale(modelGround, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelGround));
	normalMatrix = glm::mat3(glm::inverseTranspose(view * modelGround));
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	ground.Draw(shader);

}

void initFaces()
{		

	faces.push_back("textures/skybox/posx.jpg");
	faces.push_back("textures/skybox/negx.jpg");
	faces.push_back("textures/skybox/posy.jpg");
	faces.push_back("textures/skybox/negy.jpg");
	faces.push_back("textures/skybox/posz.jpg");
	faces.push_back("textures/skybox/negz.jpg");
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void initSkyBox() {
	
	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, 
		glm::value_ptr(projection));
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	simpleDepthMap.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(simpleDepthMap.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(simpleDepthMap);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		if (!panorama) {
			view = myCamera.getViewMatrix();
		}		
		else
		{			
				view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
				float radius = 10.0f;
				float camX = sin(glfwGetTime()) * radius;
				float camZ = cos(glfwGetTime()) * radius;
				view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(10.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);
	}

	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initFaces();
	initSkyBox();

	boundingBoxObj2MIN = glm::vec3(INT_MAX, INT_MAX, INT_MAX);
	boundingBoxObj2MAX = glm::vec3(INT_MIN, INT_MIN, INT_MIN);
	computeBoundingBox(cactus, boundingBoxObj2MIN, boundingBoxObj2MAX);
	std::cout << boundingBoxObj2MIN.x << " " << boundingBoxObj2MIN.y << " " << boundingBoxObj2MIN.z << " " << std::endl;
	std::cout << boundingBoxObj2MAX.x << " " << boundingBoxObj2MAX.y << " " << boundingBoxObj2MAX.z << " " << std::endl;
	boundingBoxObj1MIN = glm::vec3(INT_MAX, INT_MAX, INT_MAX);
	boundingBoxObj1MAX = glm::vec3(INT_MIN, INT_MIN, INT_MIN);
	computeBoundingBox(myModel, boundingBoxObj1MIN, boundingBoxObj1MAX);
	std::cout << boundingBoxObj1MIN.x << " " << boundingBoxObj1MIN.y << " " << boundingBoxObj1MIN.z << " " << std::endl;
	std::cout << boundingBoxObj1MAX.x << " " << boundingBoxObj1MAX.y << " " << boundingBoxObj1MAX.z << " " << std::endl;

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
