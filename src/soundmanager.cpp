#include "SoundManager.h"

bool SoundManager::s_enabled = false; //Is the sound Enabled?
bool SoundManager::s_paused = true;
char *SoundManager::s_current_sound; //current sound to play
//FMOD system stuff

FMOD_RESULT SoundManager::s_result;
FMOD_SYSTEM *SoundManager::s_fmod_system;
FMOD_SOUND *SoundManager::s_sound;
FMOD_CHANNEL *SoundManager::s_channel;

void SoundManager::Init()
{
	s_enabled = true;
	s_result = FMOD_System_Create(&s_fmod_system);
	assert(s_result == FMOD_OK);
	//Initializes the system with 1 channel
	s_result = FMOD_System_Init(s_fmod_system, 1, FMOD_INIT_NORMAL, 0);
	assert(s_result == FMOD_OK);
	FMOD_Channel_SetVolume(s_channel, 0.0f);
}
void SoundManager::Update()
{
	FMOD_System_Update(s_fmod_system);
}


void SoundManager::SetVolume(float vol)
{
	if (s_enabled && vol >= 0.0f && vol <= 1.0f)
	{
		FMOD_Channel_SetVolume(s_channel, vol);
	}
}


//loads a soundfile
void SoundManager::Load(const std::string filename)
{
	s_current_sound = (char *)filename.c_str();
	if (s_enabled)
	{
		s_result = FMOD_Sound_Release(s_sound);
		s_result = FMOD_System_CreateStream(s_fmod_system, s_current_sound, FMOD_SOFTWARE, 0, &s_sound);
		assert(s_result == FMOD_OK);
	}
}

void SoundManager::Unload(void)
{
	s_result = FMOD_Sound_Release(s_sound);
	assert(s_result == FMOD_OK);
}

void SoundManager::Play(bool pause = false) //No agument is needed to play by default
{
	if (true == s_enabled)
	{
		s_paused = pause;
		s_result = FMOD_System_PlaySound(s_fmod_system, FMOD_CHANNEL_FREE, s_sound, pause, &s_channel);
		assert(s_result == FMOD_OK);
		FMOD_Channel_SetMode(s_channel, FMOD_LOOP_NORMAL);
	}
}


bool SoundManager::GetSoundState()
{
	return s_enabled;
}

bool SoundManager::isPaused()
{
	return s_paused;
}

//Gets the name of the current sound
char* SoundManager::GetCurrentSound()
{
	return s_current_sound;
}

FMOD_SYSTEM* SoundManager::GetSystem()
{
	return s_fmod_system;
}

//pause or unpause the sound
void SoundManager::SetPause(bool pause)
{
	FMOD_Channel_SetPaused(s_channel, pause);
	s_paused = pause;
}


//turn sound on or off
void SoundManager::SetSound(bool s)
{
	s_enabled = s;
}


//toggles sound on and off
void SoundManager::ToggleSound(void)
{
	s_enabled = !s_enabled;
	if (s_enabled == true)
	{
		Load(s_current_sound);
		Play(s_enabled);
	}
	if (s_enabled == false)
	{
		Unload();
	}
}


//toggle pause on and off
void SoundManager::TogglePause(void)
{
	FMOD_BOOL p;
	s_paused = !s_paused;
	FMOD_Channel_GetPaused(s_channel, &p);
	FMOD_Channel_SetPaused(s_channel, !p);
}

SoundEffect::SoundEffect(std::string filename)
{
	b_enabled = true;
	m_sound_name = filename;
	m_result = FMOD_System_CreateSound(SoundManager::GetSystem(), m_sound_name.c_str(), FMOD_SOFTWARE, 0, &m_sound);
	assert(m_result == FMOD_OK);
}
void SoundEffect::Play()
{
	m_result = FMOD_System_PlaySound(SoundManager::GetSystem(), FMOD_CHANNEL_FREE, m_sound, false, 0);
	assert(m_result == FMOD_OK);
}