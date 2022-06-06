#include <stdio.h>
#include <iostream>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

const int k_MinMajorOpenGLVersion = 4;
const int k_MinMinorOpenGLVersion = 0;

int g_width = 1080;
int g_height = 720;

struct vec2
{
	float x;
	float y;
};

struct view
{
	float scale;
	vec2 offset;
};

int g_mode = 1;
bool g_xhair = true;

view g_ViewportRaw = { 4.0, 0.0, 0.0};
view g_ViewportSmooth = { 1.0, 0.0, 0.0};

float g_translate_speed = 0.002;
float g_scale_speed = 0.005;

struct 
{
	float scaler = 1.0f;
	float XTranslator = 0.0f;
	float YTranslator = 0.0f;
} g_ViewportControl;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 400 core\n"
	"out vec4 FragColor;\n"
	"uniform vec2 u_resolution;\n"
	"uniform vec2 u_offset;\n"
	"uniform float u_scale;\n"
	"uniform int u_mode;\n"
	"uniform bool u_xhair;\n"
	"float aspect = u_resolution.x/u_resolution.y;\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"vec3 hue2rgb(float hue)\n"
	"{\n"
	"	hue = mod(hue, 1.0f); //only use fractional part of hue, making it loop\n"
	"	float r = abs(hue * 6 - 3) - 1; //red\n"
	"	float g = 2 - abs(hue * 6 - 2); //green\n"
	"	float b = 2 - abs(hue * 6 - 4); //blue\n"
	"	vec3 rgb = vec3(r,g,b); //combine components\n"
	"	rgb = clamp(rgb, 0.0f, 1.0f); //clamp between 0 and 1\n"
	"	return rgb;\n"
	"}\n"
	"vec3 hsv(vec3 hsv)\n"
	"{\n"
	"    vec3 rgb = hue2rgb(hsv.x); //apply hue\n"
	"    rgb = (1.0f -hsv.y)*vec3(1.0f, 1.0f, 1.0f) + hsv.y*rgb; //apply saturation\n"
	"    rgb = rgb * hsv.z; //apply value\n"
	"    return rgb;\n"
	"}\n"
	
	"void main()\n"
	"{\n"
	"	vec2 uv = u_scale*(gl_FragCoord.xy/u_resolution.xy - vec2(0.5f, 0.5f));\n"
	"	uv.x *= aspect;\n"
	"	vec2 vp = uv/u_scale;\n"
	"	uv += u_mode==1 ? u_offset : vec2(0.0, 0.0f);\n"
	"	vec2 c = u_mode==1 ? uv : u_offset ;\n"
	"	vec2 z = u_mode==1 ? vec2(0.0f, 0.0) : uv;\n"
	"	int i = 0;\n"
	"	int I = 0;\n"
	"	for(i=0; i<=1024; ++i){\n"
	"		z = vec2(z.x*z.x-z.y*z.y, 2*z.x*z.y) + c;\n"
	"		if( dot(z, z) > 4) break;\n"
	"	}\n"
	"	float value = sqrt(i/1024.0);\n"
	"	vec3 color = hsv(vec3(6*value, 1.0-value, 1.0-value));\n"	
	"	FragColor = vec4(color.xyz, 1.0);\n"
	"	if( u_xhair && 0.009<length(vp) && length(vp)< 0.0095 )\n"
	"		FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
	"}\0";

//Error callback function for GLFW to report errors.
void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

//Keyboard input handling.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if( action == GLFW_PRESS || action == GLFW_RELEASE)
	{
		int sign = (action == GLFW_PRESS ? 1 : -1);
		switch (key)
		{
			case GLFW_KEY_W:
			{
				g_ViewportControl.YTranslator += sign*g_translate_speed;
				break;
			}
			case GLFW_KEY_A:
			{
				g_ViewportControl.XTranslator -= sign*g_translate_speed;
				break;
			}
			case GLFW_KEY_S:
			{
				g_ViewportControl.YTranslator -= sign*g_translate_speed;
				break;
			}
			case GLFW_KEY_D:
			{
				g_ViewportControl.XTranslator += sign*g_translate_speed;
				break;
			}
			case GLFW_KEY_LEFT_SHIFT:
			{
				g_ViewportControl.scaler += sign*g_scale_speed;
				break;
			}
			case GLFW_KEY_SPACE:
			{
				g_ViewportControl.scaler -= sign*g_scale_speed;
				break;
			}
			case GLFW_KEY_TAB:
			{
				g_mode = ( sign > 0 ? 1-g_mode : g_mode);
				break;
			}
			case GLFW_KEY_C:
			{
				g_xhair = (sign > 0 ? !g_xhair : g_xhair);
				break;
			}
		}
	}
}

void applyControl()
{
	g_ViewportRaw.scale *= g_ViewportControl.scaler;
	g_ViewportRaw.offset.x += g_ViewportRaw.scale*g_ViewportControl.XTranslator;
	g_ViewportRaw.offset.y += g_ViewportRaw.scale*g_ViewportControl.YTranslator;

	g_ViewportSmooth.scale += 0.05*( g_ViewportRaw.scale - g_ViewportSmooth.scale);
	g_ViewportSmooth.offset.x += 0.08*( g_ViewportRaw.offset.x - g_ViewportSmooth.offset.x);
	g_ViewportSmooth.offset.y += 0.08*( g_ViewportRaw.offset.y - g_ViewportSmooth.offset.y);
}

//resize handling or something idk
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	g_width = width;
	g_height = height;
	glViewport(0, 0, width, height);
}

// GEOMETRY
float triangle_vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};  

float rec_vertices[] = {
     1.0f,  1.0f, 0.0f,  // top right
     1.0f, -1.0f, 0.0f,  // bottom right
    -1.0f, -1.0f, 0.0f,  // bottom left
    -1.0f,  1.0f, 0.0f   // top left 
};

unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
};  

int main(){
	//Initialization of glfw.
	if (!glfwInit()){
		printf("Error. Could not initialize glfw.");
		return -1;
	}

	//Set an error callback so that GLFW can report any erros it runs into.
	glfwSetErrorCallback(error_callback);

	//Set minimum OpenGL version.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, k_MinMajorOpenGLVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, k_MinMinorOpenGLVersion);

	//Create windows
	GLFWwindow* window = glfwCreateWindow(g_width, g_height, "A window", NULL, NULL);

	//Error-checking
	if (!window)
	{
		printf("Error. Could not create OpenGL context. Perhaps the minimum required version is not supported.");
		return -1;
	}

	//Set the current context to the previously created window.
	glfwMakeContextCurrent(window);
	//Enable keypress handling.
	glfwSetKeyCallback(window, key_callback);
	//Enable resizing or something idk
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	//Initialize GL3W + some error checking.
	GLenum err = gl3wInit();
	if(err != GL3W_OK){
		printf("Failed to initialize GL3W");
		return -1;
	}

	// set the viewport size or something (lmao i have no idea what I'm doing)
	glViewport(0, 0, g_width, g_height);

	// something someting vertex shader whatevs
	unsigned int VBO;
	glGenBuffers(1, &VBO);  
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	
	if(!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// something something fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);


	// something something Shader Program
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::CEVA_SE_FUTU\n" << infoLog << std::endl;
	}

	int u_resolutionLocation = glGetUniformLocation(shaderProgram, "u_resolution");
	int u_scaleLocation = glGetUniformLocation(shaderProgram, "u_scale");
	int u_offsetLocation = glGetUniformLocation(shaderProgram, "u_offset");
	int u_modeLocation = glGetUniformLocation(shaderProgram, "u_mode");
	int u_xhairLocation = glGetUniformLocation(shaderProgram, "u_xhair");
	glUseProgram(shaderProgram);
	glUniform2f(u_resolutionLocation, (float)g_height, (float)g_width);

	//we can delete these after linking them
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);  
	

	//linking the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);  

	 // ..:: Initialization code :: ..
	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);
	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rec_vertices), rec_vertices, GL_STATIC_DRAW);
	// 3. copy our index array in a element buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);   

	//Main loop
	while (!glfwWindowShouldClose(window))
	{
		//Write you openGL code here.

		applyControl();

		glClearColor(0.9f, 0.3f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 4. draw the object
		glUseProgram(shaderProgram);
		glUniform2f(u_resolutionLocation, (float)g_width, (float)g_height);
		glUniform1f(u_scaleLocation, g_ViewportSmooth.scale);
		glUniform2f(u_offsetLocation, g_ViewportSmooth.offset.x, g_ViewportSmooth.offset.y);
		glUniform1i(u_modeLocation, g_mode);
		glUniform1i(u_xhairLocation, g_xhair);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		//Rendering or something idk
		glfwSwapBuffers(window);
		
		//Event checking or something idk
		glfwPollEvents();
	}

	//Terminate glfw before returning.
	glfwTerminate();
	return 0;
}
