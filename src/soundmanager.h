# pragma once

#include <vector>
#include <string>
#include <map>

#include "fmod.hpp"
#include "fmod_errors.h"

class SoundManager{
private:
	typedef struct SoundObject{
		bool isLoop;
		FMOD_VECTOR position;
		FMOD_VECTOR velocity;
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
	void init(FMOD_VECTOR startPosition = {0.f,0.f,0.f});
public:
	SoundManager(FMOD_VECTOR startPosition = { 0.f, 0.f, 0.f });
	~SoundManager();
	void setListenerPos(FMOD_VECTOR);
    int createNewChannel(std::string soundFilePath, bool isLoop, bool is3D, bool paused = false, FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f }, FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f });
	void deleteChannel(int id);
	void play(int id);
	void setPaused(int id, bool paused);
	bool isPaused(int id);
	void togglePause(int id);
	void setMicroPos(FMOD_VECTOR pos);
	void moveMicro(FMOD_VECTOR dPos);
	void setMicroVel(FMOD_VECTOR vel);
	void setMicroPosAndVel(FMOD_VECTOR pos, FMOD_VECTOR vel);
	void setSoundPos(int id, FMOD_VECTOR pos);
	void moveSound(int id, FMOD_VECTOR dPos);
	void setSoundVel(int id, FMOD_VECTOR vel);
    void setSoundPosAndVel(int id, FMOD_VECTOR pos, FMOD_VECTOR vel);
    void setMute(int id, bool mute);
    bool isMute(int id);
    void setVolume(int id, float vol);
    float getVolume(int id);
    void changeVolume(int id, float dVol);
	void update();
};