#include "soundeffect.h"
SoundEffect::SoundEffect(char* filename)
{
	b_enabled = true;
	m_sound_name = filename;
	m_result = FMOD_System_CreateSound(Sound::GetSystem(),m_sound_name, FMOD_SOFTWARE, 0, &m_sound);
	assert(m_result == FMOD_OK);
}
void SoundEffect::Play()
{
	m_result = FMOD_System_PlaySound(Sound::GetSystem(),FMOD_CHANNEL_FREE, m_sound, false, 0);
	assert(m_result == FMOD_OK);
}