#pragma once

#include "jump.h"

namespace Jump
{
    void BestTimesScoreboardMessage(edict_t* client);
    void ActiveClientsScoreboardMessage(edict_t* ent);
    void ExtendedActiveClientsScoreboardMessage(edict_t* ent);
}