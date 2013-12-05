# pragma once

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

#include "fmod.hpp"
#include "fmod_errors.h"

class SoundManager{
private:
	static void SoundManager::ERRCHECK(FMOD_RESULT);
public:
	static void start();
};