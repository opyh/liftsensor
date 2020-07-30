// StateMachine.h, a simple state machine header for C++
//
// Copyright (c) 2018 Sebastian Felix Zappe
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions: The above copyright notice
// and this permission notice shall be included in all copies or substantial portions of the
// Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "Particle.h"

template <typename StateClass>
class StateMachine
{
public:
	StateMachine(StateClass initialState, const char *_logPrefix) :
		logPrefix(_logPrefix),
		state(initialState),
		lastState(initialState),
		nextState(initialState),
		shouldDoInternalStateTransition(false),
		initialized(false),
		stateEntryTime(millis()),
		stateEntryTimeNotCountingInternalTransitions(stateEntryTime)
		{

		};

	void process()
	{
		transition(state, nextState);
		processState();
	}

	StateClass getState()
	{
		return state;
	}

protected:
	const char *logPrefix;
	StateClass state;
	StateClass lastState;
	StateClass nextState;
	bool shouldDoInternalStateTransition;
	bool initialized;
	time_t stateEntryTime;
	time_t stateEntryTimeNotCountingInternalTransitions;

	void setState(StateClass _state)
	{
		nextState = _state;
		shouldDoInternalStateTransition = true;
	}

	void transition(StateClass oldState, StateClass nextState)
	{
		if (!initialized || state != nextState || shouldDoInternalStateTransition)
		{
			if (initialized) { exitState(); }
			lastState = state;
			state = nextState;
			stateEntryTime = millis();
			if (lastState == state)
			{
				// Serial.print(".");
			}
			else
			{
				Serial.print("\r\n");
				Serial.print(logPrefix);
				Serial.print(" \033[37;1m");
				Serial.println(nameForState(nextState));
				Serial.print("\033[0m");
				stateEntryTimeNotCountingInternalTransitions = stateEntryTime;
			}
			enterState();
			shouldDoInternalStateTransition = false;
			initialized = true;
		}
	}

	void setStateAfter(StateClass _state, long after)
	{
		if (timeInCurrentState() >= after)
		{
			setState(_state);
		}
	}

	inline time_t timeInCurrentState()
	{
		return millis() - stateEntryTime;
	}

	virtual void processState() = 0;
	virtual void enterState() = 0;
	virtual void exitState() = 0;
	virtual const char *nameForState(StateClass state) = 0;
};


#endif