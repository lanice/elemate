#include "soundmanager.h"

SoundManager::SoundManager(FMOD_VECTOR startPosition){
    init(startPosition);
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

    m_channels[_id] = { isLoop, pos, vel, _channel, _sound };
    system->update();
    return _id;
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
            result = m_channels[id].channel->set3DAttributes(&m_channels[id].position, &m_channels[id].velocity);
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
                result = m_channels[id].channel->set3DAttributes(&m_channels[id].position, &m_channels[id].velocity);
                ERRCHECK(result);
                result = system->update();
                ERRCHECK(result);
                result = m_channels[id].channel->setPaused(false);
            }
        }
    }
    ERRCHECK(result);
}

void SoundManager::setPaused(int id, bool paused){
    m_channels[id].channel->setPaused(paused);
}

bool SoundManager::isPaused(int id){
    bool p;
    m_channels[id].channel->getPaused(&p);
    return p;
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
}

void SoundManager::moveMicro(FMOD_VECTOR dPos){
    position = { position.x + dPos.x, position.y + dPos.y, position.z + dPos.z };
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

void SoundManager::setMicroVel(FMOD_VECTOR vel){
    velocity = vel;
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

void SoundManager::setMicroPosAndVel(FMOD_VECTOR pos, FMOD_VECTOR vel){
    position = pos;
    velocity = vel;
    result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
    ERRCHECK(result);
}

void SoundManager::setSoundPos(int id, FMOD_VECTOR pos){
    FMOD_VECTOR _vel = m_channels[id].velocity;

    bool p;
    m_channels[id].channel->isPlaying(&p);
    if (p){
        result = m_channels[id].channel->set3DAttributes(&pos, &_vel);
    }
    ERRCHECK(result);
    m_channels[id].position = pos;
}

void SoundManager::moveSound(int id,  FMOD_VECTOR dPos){
    FMOD_VECTOR _pos = m_channels[id].position;
    FMOD_VECTOR _vel = m_channels[id].velocity;

    _pos = { _pos.x + dPos.x, _pos.y + dPos.y, _pos.z + dPos.z };

    bool p;
    m_channels[id].channel->isPlaying(&p);
    if (p){
        result = m_channels[id].channel->set3DAttributes(&_pos, &_vel);
        ERRCHECK(result);
    }
    m_channels[id].position = _pos;
}

void SoundManager::setSoundVel(int id, FMOD_VECTOR vel){
    FMOD_VECTOR _pos = m_channels[id].position;

    bool p;
    m_channels[id].channel->isPlaying(&p);
    if (p){
        result = m_channels[id].channel->set3DAttributes(&_pos, &vel);
    }
    ERRCHECK(result);
    m_channels[id].velocity = vel;
}

void SoundManager::setSoundPosAndVel(int id, FMOD_VECTOR pos, FMOD_VECTOR vel){
    bool p;
    m_channels[id].channel->isPlaying(&p);
    if (p){
        result = m_channels[id].channel->set3DAttributes(&pos, &vel);
    }
    m_channels[id].position = pos;
    m_channels[id].velocity = vel;
}

void SoundManager::setMute(int id, bool mute){
    m_channels[id].channel->setMute(mute);
}

bool SoundManager::isMute(int id){
    bool mute;
    m_channels[id].channel->getMute(&mute);
    return mute;
}

void SoundManager::setVolume(int id, float vol){
    m_channels[id].channel->setVolume(vol);
}

float SoundManager::getVolume(int id){
    float vol;
    m_channels[id].channel->getVolume(&vol);
    return vol;
}

void SoundManager::changeVolume(int id, float dVol){
    m_channels[id].channel->setVolume(getVolume(id) + dVol);
}

void SoundManager::update(){
    system->update();
}
