/****************************************************************
CNCRemote simulator
Copyright 2017 Stable Design <les@sheetcam.com>


This program is free software; you can redistribute it and/or modify
it under the terms of the Mozilla Public License Version 2.0 or later
as published by the Mozilla foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License for more details.

You should have received a copy of the Mozilla Public License
along with this program; if not, you can obtain a copy from mozilla.org
******************************************************************/

#include "sim.h"
#include <iostream>


Sim::Sim()
{
    memset(&machine, 0, sizeof(machine));
	GetState()->maxFeedLin = 1000;
}


bool Sim::Poll()
{
    if(machine.busy)
	{
		machine.busy --;
	}

	CONTROLSTATE busy = mcIDLE;
	if (machine.busy)
	{
		busy = mcMDI;
	}
	if (machine.running)
	{
		busy = mcRUNNING;
	}

	if (!machine.controlOn)
	{
		busy = mcOFF;
	}


	LockedState state = GetState();

	if (machine.jogVel.x != 0 ||
		machine.jogVel.y != 0 ||
		machine.jogVel.z != 0 ||
		machine.jogVel.a != 0 ||
		machine.jogVel.b != 0 ||
		machine.jogVel.c != 0)
	{
		if (busy == mcIDLE || busy == mcMOVING)
		{
			busy = mcMOVING;
			state->position += machine.jogVel * 1;
			machine.target = state->position;
		}
		else
		{
			machine.jogVel.Zero();
		}
	}
	Axes dtg = machine.target - state->position;
	double dist = dtg.Length();
	if (dist > 0.0001)
	{
		dtg /= dist;
		state->position += dtg * (machine.feedRate * machine.feedOverride);
		busy = mcRUNNING;
	}


	state->machineState = busy;

    if(machine.running && !machine.paused)
    {
		if (machine.runCount == 0)
		{
			machine.runCount = 1000;
			machine.curLine++;
		}else
		{
			machine.runCount--;
		}
        if(machine.step) machine.paused = true;
    }
    return(Server::Poll());
}

void Sim::UpdateState(State& state)
{
}

void Sim::DrivesOn(const bool status)
{
	std::cout << "Drives on:" << status << std::endl;

	machine.controlOn = status;
	if (!status)
	{
		LockedState state = GetState();
		machine.running = false;
		machine.jogVel.Zero();
		machine.target = state->position;
	}
}

void Sim::JogVel(const Axes velocities)
{
	ThreadLock lock = GetLock(); //Sync with main thread
	machine.jogVel = velocities;
}

void Sim::JogStep(const Axes distance, const double speed)
{
	LockedState state = GetState();
	machine.target = state->position + distance;
	machine.feedRate = speed;
}

bool Sim::Mdi(const string line)
{
	/*TODO: Proper g-code handling*/
	std::cout << "MDI:" << line << std::endl;
	LogMessage((string("MDI:") + line).c_str());
	LockedState state = GetState();
	return (state->machineState == mcIDLE || state->machineState == mcMDI);
}

void Sim::SpindleOverride(const double percent)
{
	ThreadLock lock = GetLock(); //Sync with main thread
	machine.spindleOverride = percent;

}

void Sim::FeedOverride(const double percent)
{
	ThreadLock lock = GetLock(); //Sync with main thread because a double is not atomic on a 32 bit machine
	machine.feedOverride = percent;
}

void Sim::RapidOverride(const double percent)
{

}

bool Sim::LoadFile(const string file)
{
	std::cout << "Load file:" << file << std::endl;
	return true;
}

bool Sim::CloseFile()
{
	std::cout << "Close file" << std::endl;
	return true;
}

void Sim::CycleStart()
{
	std::cout << "Cycle start" << std::endl;
}

void Sim::CycleStop()
{
	std::cout << "Cycle stop" << std::endl;
}

void Sim::FeedHold(const bool status)
{
	std::cout << "Feed hold:" << status << std::endl;
	GetState()->feedHold = status;
}

void Sim::BlockDelete(const bool status)
{
	std::cout << "Block delete:" << status << std::endl;
	GetState()->blockDelete = status;
}

void Sim::SingleStep(const bool status)
{
	std::cout << "Single step:" << status << std::endl;
	GetState()->singleStep = status;
}

void Sim::OptionalStop(const bool status)
{
	std::cout << "Optional stop:" << status << std::endl;
	GetState()->optionalStop = status;
}

void Sim::Home(const BoolAxes axes)
{
    LockedState state = GetState();
	std::cout << "Home axes:";
	for (int ct = 0; ct < MAX_AXES; ct++)
	{
		std::cout << axes.array[ct] << ",";
		if(axes.array[ct]) state->homed.array[ct] = true;
	}

	std::cout << std::endl;
}

Axes Sim::GetOffset(const unsigned int index)
{
	Axes ret;
	ret.Zero();
	return ret;
}
