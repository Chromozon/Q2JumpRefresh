#pragma once

#include "g_local.h"

namespace Jump
{

class GhostReplay
{
public:
    static void Init();
    static void LoadReplay();
    static void RunFrame();
    static void LoadGhostModels();
private:
    static std::string PickRandomGhostModel();

    static std::vector<replay_frame_t> _replay;
    static size_t _replayFrame;
    static edict_t* _ghost;
    static std::vector<std::string> _ghostModels;
};

}