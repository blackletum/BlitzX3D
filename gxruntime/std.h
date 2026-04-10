#ifndef STD_H
#define STD_H

#define _WINSOCKAPI_
#define WIN32_LEAN_AND_MEAN
#define POINTER_64 __ptr64

// OpenAL headers
#include "../openAL/include/AL/al.h"
#include "../openAL/include/AL/alc.h"
#include "../openAL/include/dr_wav.h"
#include "../openAL/include/dr_mp3.h"

#include "../fmod375/include/fmod.h"

#include "../config/config.h"
#include "../stdutil/stdutil.h"
#include "../bbruntime/constants.h"

#pragma warning( disable:4786 )

#define DIRECTSOUND_VERSION 0x700
#define NOMINMAX // stupid microsoft

#include <set>
#include <map>
#include <list>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <math.h>
#include <Windows.h>
#include <d3d9.h>

#endif