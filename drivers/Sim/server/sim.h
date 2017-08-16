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

#ifndef SIM_PLUGIN_H
#define SIM_PLUGIN_H

#include "cncserver.h"

using namespace std;
using namespace CncRemote;

class Sim : public CncRemote::Server
{
public:
    virtual void HandlePacket(const Packet & pkt);
    virtual void UpdateState();
    bool Poll();

private:
    CncRemote::Axes m_jogVel;
};

#endif

