#include "MyApp.h"
#include "GLUtils.hpp"

#include <GL/GLU.h>
#include <math.h>

// 
#include <algorithm>
#include <iomanip>

#define PRINT_XYZ(x,y,z) std::cout<<"\r" << std::setw(6) << x << "\t" << std::setw(6) << y << "\t" << std::setw(6) << z << std::flush;

bool CMyApp::InitGL()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Create VBO
	vbo = initVBO( num_particles );

	// Create particle shader
	m_program.AttachShader(GL_VERTEX_SHADER, "particle.vert");
	m_program.AttachShader(GL_GEOMETRY_SHADER, "particle.geom");
	m_program.AttachShader(GL_FRAGMENT_SHADER, "particle.frag");

	m_program.BindAttribLoc(0, "vs_in_pos");

	if (!m_program.LinkProgram())
	{
		return false;
	}

	// Load texture
	m_textureID = TextureFromFile("particle.png");

	return true;
}

bool CMyApp::InitCL()
{  
	try
	{
		///////////////////////////
		// Initialize OpenCL API //
		///////////////////////////

		cl::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		// Try to get the sharing platform!
		bool create_context_success = false;
		for (auto platform : platforms) {
			// Next, create an OpenCL context on the platform.  Attempt to
			// create a GPU-based context.
			cl_context_properties contextProperties[] =
			{
	#ifdef _WIN32
				CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
				CL_GL_CONTEXT_KHR,   (cl_context_properties)wglGetCurrentContext(),
				CL_WGL_HDC_KHR,      (cl_context_properties)wglGetCurrentDC(),
	#elif defined( __GNUC__)
				CL_CONTEXT_PLATFORM, (cl_context_properties)(platform)(),
				CL_GL_CONTEXT_KHR,   (cl_context_properties)glXGetCurrentContext(),
				CL_GLX_DISPLAY_KHR,  (cl_context_properties)glXGetCurrentDisplay(),
	#elif defined(__APPLE__)
				//todo
	#endif
				0
			};
		
			// Create Context
			try {
				context = cl::Context( CL_DEVICE_TYPE_GPU, contextProperties );
				create_context_success = true;
				break;
			} catch (cl::Error error) {}
		}
		
		if(!create_context_success)
			throw cl::Error(CL_INVALID_CONTEXT, "Failed to create CL/GL shared context");

		// Create Command Queue
		cl::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
		//std::cout << devices[0].getInfo<CL_DEVICE_NAME>() << std::endl;
		command_queue = cl::CommandQueue(context, devices[0]);

		/////////////////////////////////
		// Load, then build the kernel //
		/////////////////////////////////

		// Read source file
		std::ifstream sourceFile("GLinterop.cl");
		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length()+1));

		// Make program of the source code in the context
		program = cl::Program(context, source);
		try {
			program.build(devices);
		} catch (cl::Error error) {
			std::cout << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]) << std::endl;
			throw error;
		}

		// Make kernel
		kernel_update = cl::Kernel(program, "update");
		
		// Create Mem Objs
		cl_vbo_mem = cl::BufferGL(context, CL_MEM_WRITE_ONLY, vbo);
		cl_v = cl::Buffer(context, CL_MEM_READ_WRITE, num_particles * sizeof(float) * 3);
		cl_m = cl::Buffer(context, CL_MEM_READ_WRITE, num_particles * sizeof(float));

		///////////////////////////
		// Set-up the simulation //
		///////////////////////////

		resetSimulation();

		// kernel args
		kernel_update.setArg(0, cl_v);			// velocities
		kernel_update.setArg(1, cl_vbo_mem);	// positions
		kernel_update.setArg(2, cl_m);			// masses
		kernel_update.setArg(4, G);			    // G
	}
	catch (cl::Error error)
	{
		std::cout << error.what() << "(" << oclErrorString(error.err()) << ")" << std::endl;
		return false;
	}
	return true;
}

void CMyApp::Clean()
{
	// after we have released the OpenCL references, we can delete the underlying OpenGL objects
	if( vbo != 0 )
	{
		glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo);
		glDeleteBuffers(1, &vbo);
	}

	glDeleteTextures(1, &m_textureID);
	m_program.Clean();
}

#pragma region Update (CL)

void CMyApp::Update()
{
	if (!pause)
	{
		kernel_update.setArg(3, delta_time * time_scaler * 2);
		// CL
		try {
			cl::vector<cl::Memory> acquirable;
			acquirable.push_back(cl_vbo_mem);

			// Acquire GL Objects
			command_queue.enqueueAcquireGLObjects(&acquirable);
			{
				cl::NDRange global(num_particles);

				// interaction & integration
				command_queue.enqueueNDRangeKernel(kernel_update, cl::NullRange, global, cl::NullRange);

				// Wait for all computations to finish
				command_queue.finish();
			}
			// Release GL Objects
			command_queue.enqueueReleaseGLObjects(&acquirable);

		}
		catch (cl::Error error) {
			std::cout << error.what() << "(" << oclErrorString(error.err()) << ")" << std::endl;
			exit(1);
		}
	}
} 

#pragma endregion

#pragma region Render (GL)

void CMyApp::renderVBO( int vbolen )
{
	m_program.On();
	{
		// Shader program parameters
		m_program.SetUniform("particle_size", particle_size);
		m_program.SetTexture("tex0", 0, m_textureID);

		m_program.SetUniform("M", M);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glEnableClientState(GL_VERTEX_ARRAY);

		glDrawArrays(GL_POINTS, 0, vbolen);

		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_program.Off();
}

void CMyApp::Render()
{
	// töröljük a frampuffert (GL_COLOR_BUFFER_BIT) és a mélységi Z puffert (GL_DEPTH_BUFFER_BIT)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDisable(GL_DEPTH_TEST);	
	glDepthMask(GL_FALSE);

	// GL
	renderVBO( num_particles );
}  

#pragma endregion

#pragma region etc

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	float cameraSpeed = 0.05f;
	glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));

	switch (key.keysym.sym)
	{
	case SDLK_w:
		cameraPos += cameraSpeed * cameraFront;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		M = projection * view * model;
		break;

	case SDLK_s:
		cameraPos -= cameraSpeed * cameraFront;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		M = projection * view * model;
		break;

	case SDLK_a:
		cameraPos -= cameraRight * cameraSpeed;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		M = projection * view * model;
		break;

	case SDLK_d:
		cameraPos += cameraRight * cameraSpeed;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		M = projection * view * model;
		break;

	case SDLK_e:
		cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraRight, cameraFront));
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		M = projection * view * model;
		break;

	case SDLK_q:
		cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraRight, cameraFront));
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		M = projection * view * model;
		break;

	case SDLK_ESCAPE:
		quit = true;
		printMenu();
		break;

	case SDLK_r:
		pause = false;
		printMenu();
		break;

	case SDLK_p:
		pause = true;
		printMenu();
		break;

	case SDLK_F1:
		resetSimulation();
		printMenu();
		break;

	case SDLK_F2:
		if (++init_pos > INIT_POS_SPHERICAL)
			init_pos = INIT_POS_UNIFORM;
		initPositions();
		resetSimulation();
		printMenu();
		break;

	case SDLK_F3:
		if (++init_vel > INIT_VEL_ZERO)
			init_vel = INIT_VEL_UNIFORM;
		initVelocities();
		resetSimulation();
		printMenu();
		break;

	case SDLK_F4:
		if (++init_mas > INIT_MAS_CONSTATNS)
			init_mas = INIT_MAS_UNIFORM;
		initMasses();
		resetSimulation();
		printMenu();
		break;
	}
	
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	static float pitch = 0.0f;
	static float yaw = -90.0f;
	float sensitivity = 0.05;
	yaw += mouse.xrel * sensitivity;
	pitch -= mouse.yrel * sensitivity;

	/*
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	*/

	//std::cout << pitch << "\t" << yaw << std::endl;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); 
	front.y = sin(glm::radians(pitch)); 
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); 
	cameraFront = glm::normalize(front);

	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	M = projection * view * model;
	
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
	static float fov = 45.0f;
	fov += wheel.y;
	projection = glm::perspective(glm::radians(fov), static_cast<float>(windowW) / windowH, 0.1f, 100.0f);
	M = projection * view * model;
}

// a két paraméterbe az új ablakméret szélessége (_w) és magassága (_h) található
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h);
	windowH = _h;
	windowW = _w;
}

void CMyApp::resetSimulation()
{
	/// set masses
	command_queue.enqueueWriteBuffer(cl_m, CL_TRUE, 0, num_particles * sizeof(float), &initialMasses[0]);

	/// set initial velocities
	command_queue.enqueueWriteBuffer(cl_v, CL_TRUE, 0, num_particles * sizeof(float) * 3, &initialVelocities[0]);

	/// set initial velocities
	command_queue.enqueueWriteBuffer(cl_vbo_mem, CL_TRUE, 0, num_particles * sizeof(float) * 3, &initialPositions[0]);

	pause = true;
}

float randBetween(float min, float max) {return (rand() / float(RAND_MAX)) * (max - min) + min;}

void CMyApp::initPositions()
{
	float rMax = 1.0;
	float zMax = 1.0;

	switch (init_pos)
	{
	case INIT_POS_UNIFORM:
		generate(initialPositions.begin(), initialPositions.end(), [&]() {return randBetween(-1, 1); });
		break;

	case INIT_POS_CYLINDRICAL:

		for (int i = 0; i < num_particles * 3; i += 3)
		{
			float r = rMax * randBetween(0.5, 1.0);
			float z = zMax * randBetween(-0.5, 0.5);
			float theta = randBetween(0, 2 * M_PI);


			initialPositions[i + 0] = r * cos(theta);
			initialPositions[i + 1] = r * sin(theta);
			initialPositions[i + 2] = z;
		}
		break;

	case INIT_POS_SPHERICAL:
		for (int i = 0; i < num_particles * 3; i += 3)
		{
			float     r = rMax * randBetween(1.0, 1.0);
			float theta = randBetween(0, 2 * M_PI);
			float   phi = randBetween(0, 2 * M_PI);

			initialPositions[i + 0] = r * cos(theta) * sin(phi);
			initialPositions[i + 1] = r * sin(theta) * sin(phi);
			initialPositions[i + 2] = r * cos(phi);
		}
		break;
	}
}

void CMyApp::initVelocities()
{
	switch (init_vel)
	{
	case INIT_VEL_ZERO:
		initialVelocities = std::vector<float>(num_particles * 3, 0.0);
		break;

	case INIT_VEL_UNIFORM:
		generate(initialVelocities.begin(), initialVelocities.end(), [&]() {return randBetween(-1, 1); });
		break;
	}
}

void CMyApp::initMasses()
{

	//auto randBetween = [](float min, float max) {return (rand() / float(RAND_MAX)) * (max - min) + min; };

	switch (init_mas)
	{
	case INIT_MAS_UNIFORM:
		generate(initialVelocities.begin(), initialVelocities.end(), [&]() {return randBetween(-1, 1); });
		break;

	case INIT_MAS_CONSTATNS:
		initialVelocities = std::vector<float>(num_particles * 3, 1.0);
		break;
	}
}



void CMyApp::setQuit(const bool state)
{
	quit = state;
}

bool CMyApp::getQuit() const
{
	return quit;
}

void CMyApp::printMenu()
{
	#ifdef _WIN32
		system("cls");
	#elif defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
		system("clear");
		//add some other OSes here if needed
	#else
	#error "OS not supported."
		//you can also throw an exception indicating the function can't be used
	#endif
	std::string printedMenu = menu.str();
	printedMenu += "----------------------------------\n";

	printedMenu += "position distribution: ";
	switch (init_pos)
	{
	case INIT_POS_UNIFORM:
		printedMenu += "uniform\n";
		break;

	case INIT_POS_CYLINDRICAL:
		printedMenu += "cylindrical\n";
		break;

	case INIT_POS_SPHERICAL:
		printedMenu += "spherical\n";
		break;
	}

	printedMenu += "position distribution: ";
	switch (init_vel)
	{
	case INIT_VEL_UNIFORM:
		printedMenu += "uniform\n";
		break;

	case INIT_VEL_ZERO:
		printedMenu += "stationary\n";
		break;
	}

	printedMenu += "mass distribution: ";
	switch (init_mas)
	{
	case INIT_MAS_UNIFORM:
		printedMenu += "uniform\n";
		break;

	case INIT_MAS_CONSTATNS:
		printedMenu += "constant (" + std::to_string(initialMasses[0]) + ")\n";
		break;
	}

	printedMenu += "----------------------------------\n";

	if (pause)
		printedMenu += "Simulation paused\n";
	else
		printedMenu += "Simulation running\n";


	std::cout<<"\r" << printedMenu <<std::flush;
}

CMyApp::CMyApp(void):quit(false), pause(true), delta_time(1.0E-3), time_scaler(1.0), G(0.0001), num_particles(5000)
{
	// particles

	init_pos = INIT_POS_UNIFORM;
	init_vel = INIT_VEL_UNIFORM;
	init_mas = INIT_MAS_CONSTATNS;

	// masses
	initialMasses = std::vector<float>(num_particles, 1);
	// velocities
	initialVelocities = std::vector<float>(num_particles * 3, 0);
	// initial positions
	initialPositions = std::vector<float>(num_particles * 3, 0);

	    initMasses();
	 initPositions();
	initVelocities();


	// view
	model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), static_cast<float>(windowW) / windowH, 0.1f, 100.0f);

	cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(cameraPos, cameraPos+ cameraFront, cameraUp);

	M = projection * view * model;


	// menu
	std::cout.precision(3);
	menu = std::stringstream();
	menu << "R - Run" << std::endl;
	menu << "P - Pause" << std::endl;
	menu << "Esc - Quit" << std::endl;
	menu << "F1 - Reset" << std::endl;
	menu << "F2 - Change position distribution" << std::endl;
	menu << "F3 - Change velocity distribution" << std::endl;
	menu << "F4 - Change mass distribution" << std::endl;

	printMenu();
}

CMyApp::~CMyApp(void)
{
}

#pragma endregion
