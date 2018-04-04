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
	if (days) {
		stream.print(days, DEC);
		stream.print("d");
	}

	if (hours) {
		stream.print(hours, DEC);
		stream.print("h");
	}

	if (mins) {
		stream.print(mins, DEC);
		stream.print("m");
	}

	if (secs) {
		stream.print(secs, DEC);
		stream.print("s");
	}
}