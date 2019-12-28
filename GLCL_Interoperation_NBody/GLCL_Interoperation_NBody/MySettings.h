#pragma once

#include <iostream>
#include <fstream>

///////////////////////////////////////////////////
////    defines for particle initialization    ////
///////////////////////////////////////////////////

// position
#define INIT_POS_UNIFORM     0
#define INIT_POS_CYLINDRICAL 1
#define INIT_POS_SPHERICAL   2

// velocity
#define INIT_VEL_UNIFORM     0
#define INIT_VEL_ZERO        1

//mass
#define INIT_MAS_UNIFORM     0
#define INIT_MAS_CONSTATNS   1
///////////////////////////////////////////////////


//////////////////////////////////////////////////////
//				DEFAULT SETTINGS					//
//////////////////////////////////////////////////////

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


class MySettings
{
public:

	MySettings();
	~MySettings();

	void loadDefaultSettings();
	void set_nbParticles(unsigned int);

private:
	friend class CMyApp;
	unsigned int       windowH;
	unsigned int       windowW;
	unsigned int num_particles;
	unsigned int	  init_pos;
	unsigned int	  init_vel;
	unsigned int      init_mas;

	float  delta_time;
	float time_scaler;
	float			G;

	const float particle_size = 0.02f;

	bool  quit;
	bool pause;


};


