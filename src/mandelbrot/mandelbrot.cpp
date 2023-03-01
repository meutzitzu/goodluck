#include <stdio.h>
#include "GL/gl3w.h"
#include "GLFW/glfw3.h"
#include "bu_glw.hpp"
#include <math.h>

/* For explanations about the glfw API see minimal_triangle.cpp  */
static void glfw_error_handler(int error, const char* message);
static void glfw_key_handler(GLFWwindow*, int, int, int, int);

#define CAMERA2D_TRANSLATION_SPEED 0.05
#define CAMERA2D_TRANSLATION_SMOOTH 0.1
#define CAMERA2D_SCALE_SPEED 0.05
#define CAMERA2D_SCALE_SMOOTH 0.05
#define CAMERA2D_ROTATION_SPEED 0.05


struct {
	struct
	{
		float x = 0.0;
		float y = 0.0;
	} vpOffset;
	struct
	{
		float x = 0.0;
		float y = 0.0;
	} vpPosition;
	float vpScale = 1.0;
	float vpRotation = 0.0;
} camera;

struct
{
	bool H = false;
	bool J = false;
	bool K = false;
	bool L = false;
	bool space = false;
	bool shift = false;
}controlKeys;

int main(void){
	if (!glfwInit()){
		fprintf(stderr, "Failed to initialize glfw!\n");
		exit(1);
	}
	glfwSetErrorCallback(&glfw_error_handler);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Minimal Triangle", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Failed to create glfw window!\n");
		exit(1);
	}
	glfwSetKeyCallback(window, &glfw_key_handler);
	
	glfwMakeContextCurrent(window);
	

	if(gl3wInit()){
		fprintf(stderr, "Gl3w function loading failed!\n");
	}
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	/* For more explanations about the library API see minimal_traingle.cpp. It has comments explaining every call. */

	float verticesM[] =
	{ 	
		-1.0,  1.0, 0.0,
		 1.0,  1.0, 0.0,
		 1.0, -1.0, 0.0,
		-1.0,  1.0, 0.0,
		-1.0, -1.0, 0.0,
		 1.0, -1.0, 0.0,
	};

	float verticesJ[] =
	{ 	
		-1.0,  0.5, 0.0,
		-0.5, -0.5, 0.0,
		 0.5, -0.5, 0.0,
	};

	enum class Uniforms{ u_resolution = 0, u_time = 1, u_view = 2, u_CZ = 3 };

	VBO vbo(&verticesM);	
	VAO vao;
	vao.add_attribute(3);
	vao.bind_attributes();

	ShaderProgram prog("./vert.glsl", "./frag.glsl");

	prog.use();
	prog.registerUniform("u_resolution"); /* Resolution will have ID 1.*/
	prog.registerUniform("u_time"); /* Time will have ID 2.*/
	prog.registerUniform("u_view");
	prog.registerUniform("u_CZ");
	prog.finishUniformRegistration();

	prog.setUniform((int)Uniforms::u_resolution, (unsigned int)1920, (unsigned int)1080); /* ID 1 is VerticalOffset. Same goes for the post-fix.*/

	VBO vbo2(&verticesJ);	
	VAO vao2;
	vao2.add_attribute(3);
	vao2.bind_attributes();
	
	ShaderProgram prog2("./vert.glsl", "./fragJ.glsl");


	prog2.use();
	prog2.registerUniform("u_resolution"); /* Resolution will have ID 1.*/
	prog2.registerUniform("u_time"); /* Time will have ID 2.*/
	prog2.registerUniform("u_view");
	prog2.registerUniform("u_CZ");
	prog2.finishUniformRegistration();

	prog2.setUniform((int)Uniforms::u_resolution, (unsigned int)1920, (unsigned int)1080); /* ID 1 is VerticalOffset. Same goes for the post-fix.*/

	/* Enable Vsync*/
	glfwSwapInterval(1);
	glEnable(GL_BLEND);

	while (!glfwWindowShouldClose(window))
	{
		double t = glfwGetTime();
		float color_value = abs(sin(t));
		/* Set background color. */
		glClearColor(0.2f, 0.2f, 0.2f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT);

		prog.use();

		prog.setUniform((int)Uniforms::u_time, (float)t); /* ID 0 is the Color variable. The f post-fix must also be used, because otherwise the compiler can't deduce which function we wish to call.*/

		camera.vpOffset.x = ( camera.vpOffset.x + camera.vpScale*(-CAMERA2D_TRANSLATION_SPEED*controlKeys.H + CAMERA2D_TRANSLATION_SPEED*controlKeys.L));

		camera.vpOffset.y = ( camera.vpOffset.y + camera.vpScale*(-CAMERA2D_TRANSLATION_SPEED*controlKeys.J + CAMERA2D_TRANSLATION_SPEED*controlKeys.K));

		camera.vpPosition.x = (1.0-CAMERA2D_TRANSLATION_SMOOTH)*camera.vpPosition.x + CAMERA2D_TRANSLATION_SMOOTH*camera.vpOffset.x;
		camera.vpPosition.y = (1.0-CAMERA2D_TRANSLATION_SMOOTH)*camera.vpPosition.y + CAMERA2D_TRANSLATION_SMOOTH*camera.vpOffset.y;

		camera.vpScale *= (1.0 + CAMERA2D_SCALE_SPEED*(controlKeys.shift - controlKeys.space));

		prog.setUniform((int)Uniforms::u_view, camera.vpPosition.x, camera.vpPosition.y, camera.vpScale, camera.vpRotation); /* ID 0 is the Color variable. The f post-fix must also be used, because otherwise the compiler can't deduce which function we wish to call.*/
		


		glDrawArrays(GL_TRIANGLES, 0, 6);

		vao.bind();
		prog2.use();

		prog2.setUniform((int)Uniforms::u_time, (float)t); /* ID 0 is the Color variable. The f post-fix must also be used, because otherwise the compiler can't deduce which function we wish to call.*/

		prog2.setUniform((int)Uniforms::u_view, camera.vpPosition.x, camera.vpPosition.y, camera.vpScale, camera.vpRotation); /* ID 0 is the Color variable. The f post-fix must also be used, because otherwise the compiler can't deduce which function we wish to call.*/
		/* Draw the triangle mesh*/		


		/* Draw the triangle mesh*/		
		glDrawArrays(GL_TRIANGLES, 2, 6);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}


static void glfw_error_handler(int error, const char* message){
	fprintf(stderr, "Glfw error number %d has appeared - %s\n", error, message);
}

static void glfw_key_handler(GLFWwindow* window, int key, int scancode, int action, int mods){
	if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q ) && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	bool unused;
	bool* keyState = &unused;

	switch(key)
	{
		case GLFW_KEY_H:
			keyState = &controlKeys.H;
		break;
		case GLFW_KEY_J:
			keyState = &controlKeys.J;
		break;
		case GLFW_KEY_K:
			keyState = &controlKeys.K;
		break;
		case GLFW_KEY_L:
			keyState = &controlKeys.L;
		break;
		case GLFW_KEY_SPACE:
			keyState = &controlKeys.space;
		break;
		case GLFW_KEY_LEFT_SHIFT:
			keyState = &controlKeys.shift;
		break;
	}

	switch(action)
	{
		case GLFW_PRESS:
			*keyState = true;
		break;
		case GLFW_RELEASE:
			*keyState = false;
		break;
	}
}
