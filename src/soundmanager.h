# pragma once
#include "fmod.h"
#include "assert.h"

class SoundManager{
private:
	static bool s_enabled; //Is the sound Enabled?
	static bool s_paused; //Is the sound paused?
	static char *s_current_sound; //current sound to play
	//FMOD system stuff
	static FMOD_RESULT s_result;
	static FMOD_SYSTEM *s_fmod_system;
	static FMOD_SOUND *s_sound;
	static FMOD_CHANNEL *s_channel;
public:
	static void Init();
	static void Update();
	//Sound Control
	static void SetVolume(float vol); //Sets the volume of the playing sound
	static void Load(const char *filename);
	static void Unload();
	static void Play(bool pause);
	//getters
	static bool GetSoundState(); //checks whether the sound is on
	static bool isPaused();
	static char* GetCurrentSound();
	//setters
	static void SetPause(bool pause); //pause or unpause the sound
	static void SetSound(bool sound); //set the sound on or off


	//toggles
	static void ToggleSound(); //toggles sound on and off
	static void TogglePause(); //toggle pause on/off
};