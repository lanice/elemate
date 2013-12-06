#include "soundmanager.h"

SoundManager::SoundManager(){
	init();
}

SoundManager::~SoundManager(){
	for (SoundMap::iterator it = m_channels.begin(); it != m_channels.end(); ++it){
		result = it->second.sound->release();
		ERRCHECK(result);
	}

	result = system->close();
	ERRCHECK(result);
	result = system->release();
	ERRCHECK(result);
}

void SoundManager::ERRCHECK(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

int SoundManager::getNextFreeId(){
	int i = 0;
	while (i < m_channels.size()){
		if (m_channels.find(i) == m_channels.end()) return i;
		++i;
	}
	return i;
}

int SoundManager::createNewChannel(std::string soundFilePath, bool isLoop, bool is3D, FMOD_VECTOR pos, FMOD_VECTOR vel){
	FMOD::Sound *_sound;
	FMOD::Channel *_channel = 0;
	int _id = getNextFreeId();
	FMOD_MODE _mode;

	if (is3D)
		_mode = FMOD_3D;
	else
		_mode = FMOD_SOFTWARE | FMOD_2D;

	/** set new sound **/
	result = system->createSound(soundFilePath.c_str(), _mode, 0, &_sound);
	ERRCHECK(result);
	if (isLoop){
		result = _sound->setMode(FMOD_LOOP_NORMAL);
		ERRCHECK(result);
	}
	if (is3D){
		result = _sound->set3DMinMaxDistance(0.5f * distanceFactor, 5000.0f * distanceFactor);
		ERRCHECK(result);
	}
	result = system->playSound(FMOD_CHANNEL_FREE, _sound, true, &_channel);
	ERRCHECK(result);

	if (is3D){
		result = _channel->set3DAttributes(&pos, &vel);
		ERRCHECK(result);
	}

	if (isLoop){
		result = _channel->setPaused(false);
		ERRCHECK(result);
	}

	m_channels[_id] = { isLoop, pos, vel, _channel, _sound };
	system->update();
	return _id;
}

int SoundManager::createNewChannel(std::string soundFilePath, bool isLoop, bool is3D){
	FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
	return createNewChannel(soundFilePath, isLoop, is3D, pos, vel);
}

void SoundManager::deleteChannel(int id){
	m_channels[id].sound->release();
	ERRCHECK(result);
	m_channels.erase(id);
}

void SoundManager::play(int id){
	if (m_channels[id].isLoop){
		result = m_channels[id].channel->setPaused(false);
	}else{
		bool p;
		m_channels[id].channel->isPlaying(&p);
		if (!p){
			result = system->playSound(FMOD_CHANNEL_REUSE, m_channels[id].sound, true, &(m_channels[id].channel));
			ERRCHECK(result);
			result = m_channels[id].channel->set3DAttributes(&m_channels[id].pos, &m_channels[id].vel);
			ERRCHECK(result);
			result = m_channels[id].channel->setPaused(false);
			ERRCHECK(result);
			result = system->update();
		}else{
			bool p;
			m_channels[id].channel->getPaused(&p);
			if (p){
				result = m_channels[id].channel->setPaused(false);
			}else{
				result = m_channels[id].channel->stop();
				ERRCHECK(result);
				result = system->playSound(FMOD_CHANNEL_REUSE, m_channels[id].sound, true, &(m_channels[id].channel));
				ERRCHECK(result);
				result = m_channels[id].channel->set3DAttributes(&m_channels[id].pos, &m_channels[id].vel);
				ERRCHECK(result);
				result = system->update();
				ERRCHECK(result);
				result = m_channels[id].channel->setPaused(false);
			}
		}
	}
	ERRCHECK(result);
}

void SoundManager::pause(int id){
	m_channels[id].channel->setPaused(true);
}

void SoundManager::togglePause(int id){
	bool p;
	m_channels[id].channel->getPaused(&p);
	m_channels[id].channel->setPaused(!p);
}


void SoundManager::setMicroPos(FMOD_VECTOR pos){
	position = pos;
	result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	ERRCHECK(result);

	system->update();
}

void SoundManager::moveMicro(FMOD_VECTOR dPos){
	position = { position.x + dPos.x, position.y + dPos.y, position.z + dPos.z };
	result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	ERRCHECK(result);

	system->update();
}

void SoundManager::setMicroVel(FMOD_VECTOR vel){
	velocity = vel;
	result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	ERRCHECK(result);

	system->update();
}

void SoundManager::setMicroPosAndVel(FMOD_VECTOR pos, FMOD_VECTOR vel){
	position = pos;
	velocity = vel;
	result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	ERRCHECK(result);
	system->update();
}

void SoundManager::setSoundPos(int id, FMOD_VECTOR pos){
	FMOD_VECTOR _vel = m_channels[id].vel;

	bool p;
	m_channels[id].channel->isPlaying(&p);
	if (p){
		result = m_channels[id].channel->set3DAttributes(&pos, &_vel);
	}
	ERRCHECK(result);
	m_channels[id].pos = pos;
	system->update();
}

void SoundManager::moveSound(int id,  FMOD_VECTOR dPos){
	FMOD_VECTOR _pos = m_channels[id].pos;
	FMOD_VECTOR _vel = m_channels[id].vel;

	_pos = { _pos.x + dPos.x, _pos.y + dPos.y, _pos.z + dPos.z };

	bool p;
	m_channels[id].channel->isPlaying(&p);
	if (p){
		result = m_channels[id].channel->set3DAttributes(&_pos, &_vel);
		ERRCHECK(result);
	}
	m_channels[id].pos = _pos;
	system->update();
}

void SoundManager::setSoundVel(int id, FMOD_VECTOR vel){
	FMOD_VECTOR _pos = m_channels[id].pos;

	bool p;
	m_channels[id].channel->isPlaying(&p);
	if (p){
		result = m_channels[id].channel->set3DAttributes(&_pos, &vel);
	}
	ERRCHECK(result);
	m_channels[id].vel = vel;
	system->update();
}

void SoundManager::setSoundPosAndVel(int id, FMOD_VECTOR pos, FMOD_VECTOR vel){
	bool p;
	m_channels[id].channel->isPlaying(&p);
	if (p){
		result = m_channels[id].channel->set3DAttributes(&pos, &vel);
	}
	m_channels[id].pos = pos;
	m_channels[id].vel = vel;
}

void SoundManager::init(){
	result = FMOD::System_Create(&system);
	ERRCHECK(result);

	result = system->getVersion(&version);
	ERRCHECK(result);

	if (version < FMOD_VERSION)
	{
		printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
		return;
	}

	result = system->getNumDrivers(&numdrivers);
	ERRCHECK(result);

	if (numdrivers == 0)
	{
		result = system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		ERRCHECK(result);
	}
	else
	{
		result = system->getDriverCaps(0, &caps, 0, &speakermode);
		ERRCHECK(result);

		result = system->setSpeakerMode(speakermode);       /* Set the user selected speaker mode. */
		ERRCHECK(result);

		if (caps & FMOD_CAPS_HARDWARE_EMULATED)             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
		{                                                   /* You might want to warn the user about this. */
			result = system->setDSPBufferSize(1024, 10);
			ERRCHECK(result);
		}

		result = system->getDriverInfo(0, name, 256, 0);
		ERRCHECK(result);

		if (strstr(name, "SigmaTel"))   /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
		{
			result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
			ERRCHECK(result);
		}
	}

	result = system->init(100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)         /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
	{
		result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
		ERRCHECK(result);

		result = system->init(100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
		ERRCHECK(result);
	}


	/*
	Set the distance units. (meters/feet etc).
	*/
	result = system->set3DSettings(1.0, distanceFactor, 1.0f);
	ERRCHECK(result);

	forward = { 0.f, 0.f, 1.f };
	up = { 0.f, 1.f, 0.f };
	position = { 0.f, 0.f, 0.f };
	velocity = { 0.f, 0.f, 0.f };
	result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	ERRCHECK(result);
}