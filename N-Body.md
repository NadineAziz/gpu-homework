# N-Body simulation (GPU)

## Changes in Code

## 1. main.cpp
*	Intialized </br>
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  4);</br>

*   Set SDL_Window *win = nullptr;
*   Changed the sizes a bit</br>

	SDL_Window *win = nullptr;</br>
    win = SDL_CreateWindow( "Hello SDL&OpenGL!",	

							700,					X Coordinate	
							100,						 Y Coordinate
							ST_DEFAULT_WINOW_WIDTH,		
							ST_DEFAULT_WINOW_HEIGHT,	
							SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN SDL_WINDOW_RESIZABLE);
* Object of the app</br>
	MySettings* settings = new MySettings();</br>
	CMyApp* app = new CMyApp(win, settings);			

* Use this function to move the mouse to the given position within the window. 
	                
            SDL_WarpMouseInWindow(win, 400, 400);
	
*  Use this function to set relative mouse mode. 
	
	While the mouse is in relative mode, the cursor is hidden, and the driver will try to report continuous motion in the current window.
	Only relative motion events will be delivered, the mouse position will not change. This function will flush any pending mouse motion.
	
	    SDL_SetRelativeMouseMode(SDL_TRUE);
        while (!app->getQuit())
	{
		
		while ( SDL_PollEvent(&ev) )
		{
			switch (ev.type)
			{
			case SDL_QUIT:
				app->setQuit(true);
				break;
* some key has been pressed</br>
			case SDL_KEYDOWN:</br>
* let the app handle it except f5</br>
				
                app->KeyboardDown(ev.key);
				
* if f5 has been pressed we change the number of particles in order to do that I completely destroy the MyApp object,however this doesn't have any effects on the settings bc It is saved in other object</br>

				if (ev.key.keysym.sym == SDLK_F5)
				{
					// destroying the app
					app->Clean();
					delete app;
					SDL_SetRelativeMouseMode(SDL_FALSE);
*  get the new number of particles should be tested for fraud values

					unsigned int num = 0;
					std::cin >> num;
					// change settings
					settings->set_nbParticles(num);
* creating (allocating a pointer) a new MyApp object and using the previous settings accordingly

					app = new CMyApp(win, settings);
					app->InitGL();
					app->InitCL();
* Use this function to raise a window above other windows and set the input focus. 

					SDL_RaiseWindow(win);
				
					SDL_WarpMouseInWindow(win, 400, 400);
					SDL_SetRelativeMouseMode(SDL_TRUE);
				}
				break;
			case SDL_KEYUP:
				app->KeyboardUp(ev.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
				app->MouseDown(ev.button);
				break;
			case SDL_MOUSEBUTTONUP:
				app->MouseUp(ev.button);
				break;
			case SDL_MOUSEWHEEL:
				app->MouseWheel(ev.wheel);
				break;
			case SDL_MOUSEMOTION:
				app->MouseMove(ev.motion);
				break;
			case SDL_WINDOWEVENT:
				// resizing the window
				if ( ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )
				{
					app->Resize(ev.window.data1, ev.window.data2);
				}
				break;
			}
		}

        app->Clean();
	    delete settings;
	    delete app;

## 2. myApp.cpp
* Included: 
     #include <algorithm></br>
     #include <iomanip>	

* This is only for debugging purposes</br>
    
        #define PRINT_XYZ(x,y,z) std::cout<<"\r" << std::setw(6) << x << "\t" << std::setw(6) << y << "\t" << std::setw(6) << z << std::flush;	
       void clearConsole()	
        {	
*	The cls command IS NOT PLATFORM INDEPENDENT so I wraped it in macros to make it work on all platform	
    
	    #ifdef _WIN32	
	
         system("cls");	
	    #elif defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))	
		system("clear");	
		//add some other OSes here if needed	
    	#else	
	    #error "OS not supported."	
	    #endif	
    }	
* Generating random values between arbitary bounds	
        
        float randBetween(float min, float max) { return (rand() / float(RAND_MAX)) * (max - min) + min; }

* To Access the numParticles 

        // Create VBO	
	    vbo = initVBO( settings->num_particles );

* Create Mem Objects	

		cl_int err;	
		cl_vbo_mem = cl::BufferGL(context, CL_MEM_READ_WRITE, vbo);	
		cl_v = cl::Buffer(context, CL_MEM_READ_WRITE, settings->num_particles * sizeof(float) * 3);	
		cl_m = cl::Buffer(context, CL_MEM_READ_WRITE, settings->num_particles * sizeof(float));
        resetSimulation();

* Intialize G

         kernel_update.setArg(4, settings->G);   // G

* 	Update the update funcion

        void CMyApp::Update()	
        {	
    	if (!settings->pause)	
    	{	
	    	kernel_update.setArg(3, settings->delta_time * settings->time_scaler * 2);	
* CL

		    try {	
			    cl::vector<cl::Memory> acquirable;	
			    acquirable.push_back(cl_vbo_mem);	
* Acquire GL Objects	

			    command_queue.enqueueAcquireGLObjects(&acquirable);	
		    	{	
			    	cl::NDRange global(settings->num_particles);	
* interaction & integration	

			    	command_queue.enqueueNDRangeKernel(kernel_update, cl::NullRange, global, cl::NullRange);	
* Wait for all computations to finish	

				command_queue.finish();	
			}	
* Release GL Objects	

			command_queue.enqueueReleaseGLObjects(&acquirable);	
		}	
		catch (cl::Error error) {	
			std::cout << error.what() << "(" << oclErrorString(error.err()) << ")" << std::endl;	
			exit(1);	
		}	
	}	
	

* Shader program parameters	

		m_program.SetUniform("particle_size", settings->particle_size);	
		m_program.SetTexture("tex0", 0, m_textureID);	
		m_program.SetUniform("M", M);	
		glBindBuffer(GL_ARRAY_BUFFER, vbo);	
		glVertexPointer(3, GL_FLOAT, 0, 0);

* KeyboardDown Events function

    	void CMyApp::KeyboardDown(SDL_KeyboardEvent& key){
    	
      	// default speed of camera	
        	float cameraSpeed = 0.05f;	
		
* The missing part (unit vector that perpendicular to the other 2) of the cameras coordinate system	

	    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));	
	    switch (key.keysym.sym)	
	    {	
	    case SDLK_w:	

* move the cameras position	

		cameraPos += cameraSpeed * cameraFront;	
			
* To change it, the view of the matrix should be changed	
	
    	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);	
			
* M matrix as well	

		M = projection * view * model;	
		break;	
	    case SDLK_s:	
* same just backwards	

		cameraPos -= cameraSpeed * cameraFront;	
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);	
		M = projection * view * model;	
		break;	
    	case SDLK_a:	
*  same just it goes left	

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
* Ascend perpendicular to the direction where the camera looking at	use the right hand, the index finger is the direction where the camera points and we ascend in the direction of the thumb

		cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraRight, cameraFront));	
			
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);	
		
		M = projection * view * model;	
		break;	
	    case SDLK_q:	
* Same as before just descend	

		cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraRight, cameraFront));	
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);	
		M = projection * view * model;	
		break;	
	    case SDLK_ESCAPE:	
* Quit 

		settings->quit = true;	
		printMenu();	
		break;	
	    case SDLK_r:	
* Run that simulation	

		settings->pause = false;	
		printMenu();	
		break;	
    	case SDLK_p:	
* Pause	

		settings->pause = true;	
		printMenu();	
		break;	
	    case SDLK_F1:	
* Restore the default state of the system and reuploading the initialized parameters (masses, positions..)	

		resetSimulation();	
		printMenu();	
		break;	
	    case SDLK_F2:	
* Cycle throgh the options we can use to initialize the	positions of the particles

		if (++(settings->init_pos) > INIT_POS_SPHERICAL)	
			settings->init_pos = INIT_POS_UNIFORM;	
			
		// now we have to apply those changes	
		initPositions();	
			
		// we have to upload the new values	
		resetSimulation();	
			
* Finally we have to update the menu	

		printMenu();	
		break;	
    	case SDLK_F3:	
			
		if (++(settings->init_vel) > INIT_VEL_ZERO)	
			settings->init_vel = INIT_VEL_UNIFORM;	
		initVelocities();	
		resetSimulation();	
		printMenu();	
		break;	
	    case SDLK_F4:	
			
		if (++(settings->init_mas) > INIT_MAS_CONSTATNS)	
			settings->init_mas = INIT_MAS_UNIFORM;	
		initMasses();	
		resetSimulation();	
		printMenu();	
		break;	
	    case SDLK_F5:	
* Handle this outside	

		break;	
    	case SDLK_F6:	
		//  change g	
		// first,  clear the console	
		clearConsole();	
			
		// repeated function 	
		SDL_SetRelativeMouseMode(SDL_FALSE);	
			
		// click in the console to enter the value	
		std::cout << "Click here and enter the desired value: ";	
		float temp1;	
		std::cin >> temp1;	
			
		//  tests	
		if (temp1 > 0.0)	
			settings->G = temp1;	
		// raise awareness	
		if (settings->G > 0.1)	
			extraInfo = "[Warning] Too large values may cause undesired behavior!\n";	
		else	
			extraInfo = "";	
			
*  Inform OpenCL about our changes	

		kernel_update.setArg(4, settings->G);	
		SDL_RaiseWindow(win);	
		SDL_WarpMouseInWindow(win, 400, 400);	
		SDL_SetRelativeMouseMode(SDL_TRUE);	
		printMenu();	
		break;	
    	}	
* MouseMove function

         void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)	        
            {	
            static float pitch = 0.0f;	
            static float yaw = -90.0f;	
            float sensitivity = 0.05;	
            yaw += mouse.xrel * sensitivity;	
            pitch -= mouse.yrel * sensitivity;	
          	
         
* Using math to rotate the coordinate system	

            glm::vec3 front;	
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));	

            front.y = sin(glm::radians(pitch)); 	

            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); 	

* Because it is a unit vector we have to normalize it	

            cameraFront = glm::normalize(front);	
            // same as usual	
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);	
            M = projection * view * model;	

            }	
* MouseWheel function 

        void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)	
        {	
            // change the field of view when we zoom in or out	
            static float fov = 45.0f;	
            fov += wheel.y; // -1 or +1	
            // change the projection matrix	
            projection = glm::perspective(glm::radians(fov), static_cast<float>(settings->windowW) / settings->windowH, 0.1f, 100.0f);	
            M = projection * view * model;	
        }

* Resize Function

        void CMyApp::Resize(int _w, int _h)	
        {	
            // change the size of the window	
            glViewport(0, 0, _w, _h);	
            settings->windowH = _h;	
            settings->windowW = _w;	
        }	

* Reset Simulation function

        void CMyApp::resetSimulation()	
        {	
*  uploading to the gpu	

            // set masses	
            command_queue.enqueueWriteBuffer(cl_m, CL_TRUE, 0, settings->num_particles * sizeof(float), &initialMasses[0]);	
            /// set initial velocities	
            command_queue.enqueueWriteBuffer(cl_v, CL_TRUE, 0, settings->num_particles * sizeof(float) * 3, &initialVelocities[0]);

* Set initial positions	

            command_queue.enqueueWriteBuffer(cl_vbo_mem, CL_TRUE, 0, settings->num_particles * sizeof(float) * 3, &initialPositions[0]);	
            settings->pause = true;	
        }	
* Positions

        void CMyApp::initPositions()	
        {	
            float rMax = 1.0;	
            float zMax = 1.0;	
            switch (settings->init_pos)	
            {	
            case INIT_POS_UNIFORM:	
*  The generate() function which is in the algorithm header is used for intializing. This function wants 2 pointers. the head and the tail of the array or vector. Then you have to pass a function pointer which has no arguments. Mine has so I wrap it in a lambda expression.	
Got it from: </br>
 generate( ):	 https://www.geeksforgeeks.org/stdgenerate-in-c/	
                    	

                generate(initialPositions.begin(), initialPositions.end(), [&]() {return randBetween(-1, 1); });	
                break;	
            case INIT_POS_CYLINDRICAL:	
                // we place the particles in a cylinder	
                // we looping over the particles	
                // we step 3 bc of x y z coordinates	
                	
                for (int i = 0; i < settings->num_particles * 3; i += 3)	
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
            
                for (int i = 0; i < settings->num_particles * 3; i += 3)	
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
* Velocities

        void CMyApp::initVelocities()	
        {	
            switch (settings->init_vel)	
            {	
            case INIT_VEL_ZERO:	
             
                initialVelocities = std::vector<float>(settings->num_particles * 3, 0.0);	
                break;	
            case INIT_VEL_UNIFORM:	
                generate(initialVelocities.begin(), initialVelocities.end(), [&]() {return randBetween(-1, 1); });	
                break;	
            }	
        }	
* Mass
        void CMyApp::initMasses()	
        {	
            
            switch (settings->init_mas)	
            {	
            case INIT_MAS_UNIFORM:	
                generate(initialVelocities.begin(), initialVelocities.end(), [&]() {return randBetween(-1, 1); });	
                break;	
            case INIT_MAS_CONSTATNS:	
                initialVelocities = std::vector<float>(settings->num_particles * 3, 1.0);	
                break;	
            }	
        }	

* Set Quit

            void CMyApp::setQuit(const bool state)	
        {	
            settings->quit = state;	
        }	
* Get Quit

        bool CMyApp::getQuit() const	
        {	
            return settings->quit;	
        }	
* Print Menu

        void CMyApp::printMenu()	
        {	
           
            clearConsole();	
            std::string printedMenu = menu.str();	
            printedMenu += "----------------------------------\n";	
            printedMenu += "position distribution: ";	
            switch (settings->init_pos)	
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
            switch (settings->init_vel)	
            {	
            case INIT_VEL_UNIFORM:	
                printedMenu += "uniform\n";	
                break;	
            case INIT_VEL_ZERO:	
                printedMenu += "stationary\n";	
                break;	
            }	
            printedMenu += "mass distribution: ";	
            switch (settings->init_mas)	
            {	
            case INIT_MAS_UNIFORM:	
                printedMenu += "    uniform\n";	
                break;	
            case INIT_MAS_CONSTATNS:	
                printedMenu += "    constant (" + std::to_string(initialMasses[0]) + ")\n";	
                break;	
            }	
            printedMenu += "number of particles:   " + std::to_string(settings->num_particles) + "\n";	
            printedMenu += "G:                     " + std::to_string(settings->G) + "\n";	
            printedMenu += "----------------------------------\n";	
            if (settings->pause)	
                printedMenu += "Simulation paused\n";	
            else	
                printedMenu += "Simulation running\n";	
            printedMenu += extraInfo;	
            std::cout<<"\r" << printedMenu <<std::flush;	
        }	

* Constructor

        CMyApp::CMyApp(SDL_Window* window, MySettings* _settings):win(window), settings(_settings), extraInfo()	
        {	

* Masses	

            // create a vector that has settings->num_particles elements and all of them 	
            // initialized as 1	
            initialMasses = std::vector<float>(settings->num_particles, 1);	
                
* Velocities

            // create a vector that has settings->num_particles * 3 elements (xyz) and all of them	
            // initialized as 0	
            initialVelocities = std::vector<float>(settings->num_particles * 3, 0);	
                
* Initial positions	
     
            initialPositions = std::vector<float>(settings->num_particles * 3, 0);	

* After we created the initial parameters we change them according to the default settings	
            initMasses();	
            initPositions();	
            initVelocities();	
* View	

            //  set the default viewport	
           
            model = glm::mat4(1.0f);	
            view = glm::mat4(1.0f);	
            projection = glm::perspective(glm::radians(45.0f), static_cast<float>(settings->windowW) / settings->windowH, 0.1f, 100.0f);	
            cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);	
            cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // should be unit vector	
            cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // same	
            view = glm::lookAt(cameraPos, cameraPos+ cameraFront, cameraUp); 
            // create  matrix that creates the illusion	
            M = projection * view * model;	
* Menu 	

            // I append the items in a stringstream and then I flush it onto the console	
            std::cout.precision(3); // 3 digit print	
            menu = std::stringstream();	
            menu << "R - Run" << std::endl;	
            menu << "P - Pause" << std::endl;	
            menu << "Esc - Quit" << std::endl;	
            menu << "F1 - Reset" << std::endl;	
            menu << "F2 - Change position distribution" << std::endl;	
            menu << "F3 - Change velocity distribution" << std::endl;	
            menu << "F4 - Change mass distribution" << std::endl;	
            menu << "F5 - Change the number of particles" << std::endl;	
            menu << "F6 - Change G" << std::endl;	
            printMenu();	
        }


## 3. myApp.h

* Included: 

        	// GLM
            #include <glm/glm.hpp>
            #include <glm/gtx/transform.hpp>
            #include "MySettings.h"
* Including Contructor of CMyApp

        CMyApp(SDL_Window*, MySettings*);

* Uploading the simulation parameters to the gpu

        void resetSimulation();
        // initializing the positions of the particles
        void initPositions();
        
        void initVelocities();
        
        void initMasses();
        
        // setting the value of the quit variable
        void setQuit(const bool);
        // getting the value of quit
        bool getQuit() const;
        // printing my menu
        void printMenu();

* In protected class

        	// they hold the initial values before we upload
            std::vector<float> initialMasses;
            std::vector<float> initialPositions;
            std::vector<float> initialVelocities;

* The final M matrix wich is used in particle.vert vertex shader to create the illusion of 3D motion

            glm::mat4 M;
            // coordinate systems
           
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
* They are for the camera and Its coordinate system when we move we change these variables

            glm::vec3 cameraPos;
            glm::vec3 cameraFront;
            glm::vec3 cameraUp;
            // It holds the pointer to the current sdl window
            SDL_Window* win;
            
            // this is for error message for example when you change G too big
            std::string extraInfo;
            
            // all menu items merged in this string
            std::stringstream menu;
            // this points to the settings object
            MySettings* settings;


* Helper Functions

        #pragma region Helper functions
            // Helper function to get OpenCL error string from constant
           
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

## 4. MySetting.h

#pragma once
* Include

        #include <iostream>
        #include <fstream>

* Defines for particle initialization 


* Position

        #define INIT_POS_UNIFORM     0
        #define INIT_POS_CYLINDRICAL 1
        #define INIT_POS_SPHERICAL   2

* Velocity

        #define INIT_VEL_UNIFORM     0
        #define INIT_VEL_ZERO        1

* Mass

        #define INIT_MAS_UNIFORM     0
        #define INIT_MAS_CONSTATNS   1


* DEFAULT SETTINGS	

        #define ST_DEFAULT_WINOW_WIDTH					    800		// window width
        #define ST_DEFAULT_WINOW_HEIGHT					    800		// window height
        #define ST_DEFAULT_NUM_PARTICLES				  10000		// number of particles
        #define ST_DEFAULT_POS_DIST			   INIT_POS_UNIFORM		// initial position distribution
        #define ST_DEFAULT_VEL_DIST			   INIT_VEL_UNIFORM		// initial velocity distribution
        #define ST_DEFAULT_MAS_DIST			   INIT_MAS_UNIFORM		// initial mass distribution
        #define ST_DEFAULT_DT							 1.0E-3		// default time step
        #define ST_DEFAULT_TSC							    1.0		// default time scaler
        #define ST_DEFAULT_G							 0.0001		// default G
        #define ST_DEFAULT_QUIT							  false		// default quit
        #define ST_DEFAULT_PAUSE						   true		// default pause

* Class MySettings
   
        class MySettings
            {
        public:

            MySettings();
            ~MySettings();

            void loadDefaultSettings();
            void set_nbParticles(unsigned int);
          private:

* We mark CMyApp class as friend so It can chagnes the private variables of this class

        friend class CMyApp;
        
        unsigned int       windowH; // height of the window
        unsigned int       windowW; // width ..
        unsigned int num_particles; // number of particles
        unsigned int	  init_pos; // this holds the type of the initial distribution for positions
        unsigned int	  init_vel; // same for velocities
        unsigned int      init_mas; // ..

        float  delta_time; // time step
        float time_scaler; // when we want to speed up or slow down the simulation we change this variable
        float			G; // global constant for calculating stuff

        const float particle_size = 0.02f; // the size of the particles, we dont change this so it stays constant

        bool  quit; // this indicates when we want to quit this madness
        bool pause; // we want to pause the simulation


    }

* Screenshot of Output 

![image info]("C:\Users\Nadeen\Desktop\82524046_2574572552762750_7511061377297940480_n.png")




