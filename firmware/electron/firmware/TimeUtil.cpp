#include "Particle.h"
#include "TimeUtil.h"

void printTime(Stream &stream, time_t ms)
{
	time_t secs = ms / 1000;
	time_t mins = secs / 60;
	time_t hours = mins / 60;
	time_t days = hours / 24;
	secs = secs % 60;
	mins = mins % 60;
	hours = hours % 24;
	stream.print(days, DEC);
	stream.print("d ");
	if (hours < 10) { stream.print("0"); }
	stream.print(hours, DEC);
	stream.print(":");
	if (mins < 10) { stream.print("0"); }
	stream.print(mins, DEC);
	stream.print(":");
	if (secs < 10) { stream.print("0"); }
	stream.print(secs, DEC);
}