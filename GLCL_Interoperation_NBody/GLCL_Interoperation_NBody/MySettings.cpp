#include "MySettings.h"

MySettings::MySettings() { loadDefaultSettings(); }

MySettings::~MySettings() {}

void MySettings::loadDefaultSettings()
{
	windowH       = ST_DEFAULT_WINOW_WIDTH;
	windowW       = ST_DEFAULT_WINOW_HEIGHT;
	num_particles = ST_DEFAULT_NUM_PARTICLES;
	init_pos      = ST_DEFAULT_POS_DIST;
	init_vel      = ST_DEFAULT_VEL_DIST;
	init_mas      = ST_DEFAULT_MAS_DIST;

	delta_time    = ST_DEFAULT_DT;
	time_scaler   = ST_DEFAULT_TSC;
	G             = ST_DEFAULT_G;

	quit          = ST_DEFAULT_QUIT;
	pause         = ST_DEFAULT_PAUSE;
}

void MySettings::set_nbParticles(unsigned int _nb_particles)
{
	num_particles = _nb_particles;
}
