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
				Serial.print(".");
			}
			else
			{
				Serial.print("\n");
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