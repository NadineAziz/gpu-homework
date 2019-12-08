#pragma once

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// Utils
#include "gVertexBuffer.h"
#include "gShaderProgram.h"

// CL
#include <iostream>
#include <fstream>
#include <sstream>



// GLM
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#define __NO_STD_VECTOR
#define __CL_ENABLE_EXCEPTIONS

#ifdef __APPLE__
#include <CL/cl.hpp>
#else
#include <CL/cl.hpp>
#include <CL/cl_gl.h>
#endif

#ifdef __GNUC__
#include <GL/glx.h>
#endif

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool InitGL();
	bool InitCL();

	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);

	void resetSimulation();

	void setQuit(const bool);
	bool getQuit() const;
protected:

	// GL
	int windowH, windowW;
	GLuint m_vaoID, vbo, texture;
	void renderVBO( int vbolen );

	// CL
	cl::Context context;
	cl::CommandQueue command_queue;
	cl::Program program;

	cl::Kernel kernel_update;

	cl::BufferGL cl_vbo_mem;
	cl::Buffer cl_v, cl_m;


	int num_particles = 5000;
	const float particle_size = 0.02f;
	const bool bRing = true;
	const bool bRandVelocities = false;


	std::vector<float> initialMasses;
	std::vector<float> initialPositions;
	std::vector<float> initialVelocities;

	bool quit;
	bool pause;
	float delta_time;
	float time_scaler;
	float G;


	glm::mat4 M;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

#pragma region GL functions

	gShaderProgram	m_program;
	GLuint m_textureID;

	GLuint initVBO(int vbolen )
	{
		GLuint vbo_buffer;
		// generate the buffer
		glGenBuffers(1, &vbo_buffer);

		// bind the buffer
		glBindBuffer(GL_ARRAY_BUFFER, vbo_buffer);
		glBufferData(GL_ARRAY_BUFFER, vbolen*sizeof(float)*2, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); 

		return vbo_buffer;
	}

#pragma endregion

#pragma region Helper functions

	// Helper function to get OpenCL error string from constant
	// *********************************************************************
	const char* oclErrorString(cl_int error)
	{
		static const char* errorString[] = {
			"CL_SUCCESS",
			"CL_DEVICE_NOT_FOUND",
			"CL_DEVICE_NOT_AVAILABLE",
			"CL_COMPILER_NOT_AVAILABLE",
			"CL_MEM_OBJECT_ALLOCATION_FAILURE",
			"CL_OUT_OF_RESOURCES",
			"CL_OUT_OF_HOST_MEMORY",
			"CL_PROFILING_INFO_NOT_AVAILABLE",
			"CL_MEM_COPY_OVERLAP",
			"CL_IMAGE_FORMAT_MISMATCH",
			"CL_IMAGE_FORMAT_NOT_SUPPORTED",
			"CL_BUILD_PROGRAM_FAILURE",
			"CL_MAP_FAILURE",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"CL_INVALID_VALUE",
			"CL_INVALID_DEVICE_TYPE",
			"CL_INVALID_PLATFORM",
			"CL_INVALID_DEVICE",
			"CL_INVALID_CONTEXT",
			"CL_INVALID_QUEUE_PROPERTIES",
			"CL_INVALID_COMMAND_QUEUE",
			"CL_INVALID_HOST_PTR",
			"CL_INVALID_MEM_OBJECT",
			"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
			"CL_INVALID_IMAGE_SIZE",
			"CL_INVALID_SAMPLER",
			"CL_INVALID_BINARY",
			"CL_INVALID_BUILD_OPTIONS",
			"CL_INVALID_PROGRAM",
			"CL_INVALID_PROGRAM_EXECUTABLE",
			"CL_INVALID_KERNEL_NAME",
			"CL_INVALID_KERNEL_DEFINITION",
			"CL_INVALID_KERNEL",
			"CL_INVALID_ARG_INDEX",
			"CL_INVALID_ARG_VALUE",
			"CL_INVALID_ARG_SIZE",
			"CL_INVALID_KERNEL_ARGS",
			"CL_INVALID_WORK_DIMENSION",
			"CL_INVALID_WORK_GROUP_SIZE",
			"CL_INVALID_WORK_ITEM_SIZE",
			"CL_INVALID_GLOBAL_OFFSET",
			"CL_INVALID_EVENT_WAIT_LIST",
			"CL_INVALID_EVENT",
			"CL_INVALID_OPERATION",
			"CL_INVALID_GL_OBJECT",
			"CL_INVALID_BUFFER_SIZE",
			"CL_INVALID_MIP_LEVEL",
			"CL_INVALID_GLOBAL_WORK_SIZE",
		};

		const int errorCount = sizeof(errorString) / sizeof(errorString[0]);

		const int index = -error;

		return (index >= 0 && index < errorCount) ? errorString[index] : "Unspecified Error";
	} 

#pragma endregion

};