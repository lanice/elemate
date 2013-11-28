#pragma once

#include "fmod.h"
#include "assert.h"

class SoundEffect
{
private:
	bool b_enabled;
	char* m_sound_name;
	FMOD_SOUND* m_sound;
	FMOD_RESULT m_result;
public:
	SoundEffect(char* filename);
	void Play();
};