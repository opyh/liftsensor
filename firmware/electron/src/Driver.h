#ifndef DRIVER_H
#define DRIVER_H

#include "Particle.h"


class Driver
{
	virtual void init() = 0;
	virtual void process(time_t now) = 0;
};

#endif
