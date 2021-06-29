#pragma once

#ifdef _WIN32
#define q_exported          __declspec(dllexport)
#else
#define q_exported
#endif
