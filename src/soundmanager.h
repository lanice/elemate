# pragma once

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <vector>
#include <string>
#include <map>

#include "fmod.hpp"
#include "fmod_errors.h"

class SoundManager{
private:
	typedef struct SoundObject{
		bool isLoop;
		FMOD_VECTOR pos;
		FMOD_VECTOR vel;
		FMOD::Channel *channel;
		FMOD::Sound *sound;
	}SoundObject;
	typedef std::map<int, SoundObject> SoundMap;

	FMOD_VECTOR						position;
	FMOD_VECTOR						velocity;
	FMOD_VECTOR						up;
	FMOD_VECTOR						forward;

	unsigned int					version;
	int								numdrivers;
	FMOD_SPEAKERMODE				speakermode;
	FMOD_CAPS						caps;
	char							name[256];
	float							distanceFactor = 10.f; // units per meter (centimeters = 100)
	SoundMap						m_channels;
	FMOD_RESULT						result;
	FMOD::System					*system;
	void SoundManager::ERRCHECK(FMOD_RESULT);
	int getNextFreeId();
	void init();
public:
	SoundManager();
	~SoundManager();
	void setListenerPos(FMOD_VECTOR);
	int createNewChannel(std::string soundFilePath, bool isLoop, bool is3D);
	int createNewChannel(std::string soundFilePath, bool isLoop, bool is3D, FMOD_VECTOR pos, FMOD_VECTOR vel);
	void deleteChannel(int id);
	void play(int id);
	void pause(int id);
	void togglePause(int id);
	void setMicroPos(FMOD_VECTOR pos);
	void moveMicro(FMOD_VECTOR dPos);
	void setMicroVel(FMOD_VECTOR vel);
	void setMicroPosAndVel(FMOD_VECTOR pos, FMOD_VECTOR vel);
	void setSoundPos(int id, FMOD_VECTOR pos);
	void moveSound(int id, FMOD_VECTOR dPos);
	void setSoundVel(int id, FMOD_VECTOR vel);
	void setSoundPosAndVel(int id, FMOD_VECTOR pos, FMOD_VECTOR vel);
};