#include "soundmanager.h"

/* constructor */
SoundManager::SoundManager(FMOD_VECTOR startPosition){
    init(startPosition);
}

/* destructor */
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

/* inits the SoundManager with optional start position*/
void SoundManager::init(FMOD_VECTOR startPosition){
    result = FMOD::System_Create(&system);
    ERRCHECK(result);

    result = system->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION){
        printf("Error!  You are using an old version of FMOD %08x.  This program requires %08x\n", version, FMOD_VERSION);
        return;
    }

    result = system->getNumDrivers(&numdrivers);
    ERRCHECK(result);

    if (numdrivers == 0){
        result = system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }else{
        result = system->getDriverCaps(0, &caps, 0, &speakermode);
        ERRCHECK(result);

        /** set the user selected speaker mode. **/
        result = system->setSpeakerMode(speakermode);
        ERRCHECK(result);

        /** if the user has the 'Acceleration' slider set to off!  (bad for latency) **/
        if (caps & FMOD_CAPS_HARDWARE_EMULATED){
            /** maybe print pout a warning? **/
            result = system->setDSPBufferSize(1024, 10);
            ERRCHECK(result);
        }

        result = system->getDriverInfo(0, name, 256, 0);
        ERRCHECK(result);

        /** Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. **/
        if (strstr(name, "SigmaTel"))
        {
            result = system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }
    }

    /** init system with maximum of 100 channels **/
    result = system->init(100, FMOD_INIT_NORMAL, 0);
    /** if the speaker mode selected isn't supported by this soundcard, switch it back to stereo **/
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);

        result = system->init(100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        ERRCHECK(result);
    }

    result = system->set3DSettings(1.0, distanceFactor, 1.0f);
    ERRCHECK(result);

    forward = { 0.f, 0.f, 1.f };
    up = { 0.f, 1.f, 0.f };
    position = startPosition;
    velocity = { 0.f, 0.f, 0.f };
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

/* checks errors */
void SoundManager::ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}

/* returns the next free channelId */
int SoundManager::getNextFreeId(){
    int i = 0;
    while (i < m_channels.size()){
        if (m_channels.find(i) == m_channels.end()) return i;
        ++i;
    }
    return i;
}

/* creates a new channel and returns the channelId */
int SoundManager::createNewChannel(std::string soundFilePath, bool isLoop, bool is3D, bool paused, FMOD_VECTOR pos, FMOD_VECTOR vel){
    FMOD::Sound *_sound;
    FMOD::Channel *_channel = 0;
    int _id = getNextFreeId();
    FMOD_MODE _mode;

    if (is3D){
        _mode = FMOD_3D;
    }else{
        _mode = FMOD_SOFTWARE | FMOD_2D;
    }

    /** create the sound **/
    result = system->createSound(soundFilePath.c_str(), _mode, 0, &_sound);
    ERRCHECK(result);

    /** set sound properties **/
    if (is3D){
        result = _sound->set3DMinMaxDistance(0.5f * distanceFactor, 5000.0f * distanceFactor);
        ERRCHECK(result);
    }

    if (isLoop){
        result = _sound->setMode(FMOD_LOOP_NORMAL);
        ERRCHECK(result);
    }

    /** create the channel **/
    result = system->playSound(FMOD_CHANNEL_FREE, _sound, true, &_channel);
    ERRCHECK(result);

    /** set channel properties **/
    if (is3D){
        result = _channel->set3DAttributes(&pos, &vel);
        ERRCHECK(result);
    }

    if (!paused){
        result = _channel->setPaused(false);
        ERRCHECK(result);
    }

    m_channels[_id] = { isLoop, is3D, pos, vel, _channel, _sound };
    system->update();
    return _id;
}

/* deletes a channel */
void SoundManager::deleteChannel(int channelId){
    m_channels[channelId].sound->release();
    ERRCHECK(result);
    m_channels.erase(channelId);
}

/* unpauses a loop-channel, */
/* unpauses a paused non-loop-channel, otherwise plays it from the beginning */
void SoundManager::play(int channelId){
    bool _play;
    bool _paused;
    m_channels[channelId].channel->isPlaying(&_play);
    m_channels[channelId].channel->getPaused(&_paused);
    if (_play && _paused){
        result = m_channels[channelId].channel->setPaused(false);
    }else{
        result = m_channels[channelId].channel->stop();
        ERRCHECK(result);
        if (m_channels[channelId].is3D){
            result = system->playSound(FMOD_CHANNEL_REUSE, m_channels[channelId].sound, true, &(m_channels[channelId].channel));
            ERRCHECK(result);
            result = m_channels[channelId].channel->set3DAttributes(&m_channels[channelId].position, &m_channels[channelId].velocity);
            ERRCHECK(result);
            result = system->update();
            ERRCHECK(result);
        result = m_channels[channelId].channel->setPaused(false);
        }else{
            result = system->playSound(FMOD_CHANNEL_REUSE, m_channels[channelId].sound, false, &(m_channels[channelId].channel));
        }
    }
    ERRCHECK(result);
}

/* pauses/unpauses a channel */
void SoundManager::setPaused(int channelId, bool paused){
    m_channels[channelId].channel->setPaused(paused);
}

/* returns true if the channel is paused */
bool SoundManager::isPaused(int channelId){
    bool p;
    m_channels[channelId].channel->getPaused(&p);
    return p;
}

/* toggles the pause status of channel */
void SoundManager::togglePause(int channelId){
    bool p;
    m_channels[channelId].channel->getPaused(&p);
    m_channels[channelId].channel->setPaused(!p);
}

/* sets the position of the listener (microphone) */
void SoundManager::setMicroPos(FMOD_VECTOR pos){
    position = pos;
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

/* changes the position of the listener (microphone) by dPos */
void SoundManager::moveMicro(FMOD_VECTOR dPos){
    position = { position.x + dPos.x, position.y + dPos.y, position.z + dPos.z };
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

/* sets the velocity of the listener (microphone) */
void SoundManager::setMicroVel(FMOD_VECTOR vel){
    velocity = vel;
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

/* sets position and velocity of the listener (microphone) */
void SoundManager::setMicroPosAndVel(FMOD_VECTOR pos, FMOD_VECTOR vel){
    position = pos;
    velocity = vel;
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

/* sets position of a channel in the world */
void SoundManager::setSoundPos(int channelId, FMOD_VECTOR pos){
    FMOD_VECTOR _vel = m_channels[channelId].velocity;

    bool p;
    m_channels[channelId].channel->isPlaying(&p);
    if (p){
        result = m_channels[channelId].channel->set3DAttributes(&pos, &_vel);
    }
    ERRCHECK(result);
    m_channels[channelId].position = pos;
}

void SoundManager::moveSound(int channelId, FMOD_VECTOR dPos){
    FMOD_VECTOR _pos = m_channels[channelId].position;
    FMOD_VECTOR _vel = m_channels[channelId].velocity;

    _pos = { _pos.x + dPos.x, _pos.y + dPos.y, _pos.z + dPos.z };

    bool p;
    m_channels[channelId].channel->isPlaying(&p);
    if (p){
        result = m_channels[channelId].channel->set3DAttributes(&_pos, &_vel);
        ERRCHECK(result);
    }
    m_channels[channelId].position = _pos;
}

/* sets the velocity of a channel */
void SoundManager::setSoundVel(int channelId, FMOD_VECTOR vel){
    FMOD_VECTOR _pos = m_channels[channelId].position;

    bool p;
    m_channels[channelId].channel->isPlaying(&p);
    if (p){
        result = m_channels[channelId].channel->set3DAttributes(&_pos, &vel);
    }
    ERRCHECK(result);
    m_channels[channelId].velocity = vel;
}

/* sets positions and velocity of a channel in the world */
void SoundManager::setSoundPosAndVel(int channelId, FMOD_VECTOR pos, FMOD_VECTOR vel){
    bool p;
    m_channels[channelId].channel->isPlaying(&p);
    if (p){
        result = m_channels[channelId].channel->set3DAttributes(&pos, &vel);
    }
    m_channels[channelId].position = pos;
    m_channels[channelId].velocity = vel;
}

/* mutes or unmutes the channel */
void SoundManager::setMute(int channelId, bool mute){
    m_channels[channelId].channel->setMute(mute);
}

/* returns true if the channel is mute */
bool SoundManager::isMute(int channelId){
    bool mute;
    m_channels[channelId].channel->getMute(&mute);
    return mute;
}

/* sets volume of a channel*/
void SoundManager::setVolume(int channelId, float vol){
    m_channels[channelId].channel->setVolume(vol);
}

/* returns the current volume of a channel*/
float SoundManager::getVolume(int channelId){
    float vol;
    m_channels[channelId].channel->getVolume(&vol);
    return vol;
}

/* changes volume of a specific channel by dVol (should be between -1.f and 1.f) */
void SoundManager::changeVolume(int channelId, float dVol){
    m_channels[channelId].channel->setVolume(getVolume(channelId) + dVol);
}

/* changes the played file of a channel */
void SoundManager::changeFile(int channelId, std::string filePath){
    FMOD_MODE _mode;
    bool _ispaused;
    /* get current sound properties */
    m_channels[channelId].channel->getPaused(&_ispaused);
    m_channels[channelId].sound->getMode(&_mode);
    /* stop and release current sound object */
    m_channels[channelId].channel->stop();
    m_channels[channelId].sound->release();

    /* create new sound object */
    result = system->createSound(filePath.c_str(), _mode, 0, &(m_channels[channelId].sound));
    ERRCHECK(result);
    /* bind it to the channel */
    result = system->playSound(FMOD_CHANNEL_REUSE, m_channels[channelId].sound, _ispaused, &(m_channels[channelId].channel));
    ERRCHECK(result);
}

/* updates 3D positions */
void SoundManager::update(){
    system->update();
}
