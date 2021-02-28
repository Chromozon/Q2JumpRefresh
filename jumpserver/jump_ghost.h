#pragma once

#include "g_local.h"

namespace Jump
{
    void GhostChangeReplay();
    void GhostRunFrame();
    void GhostInit(edict_t* ghost);
    edict_t* GhostInstance();
}