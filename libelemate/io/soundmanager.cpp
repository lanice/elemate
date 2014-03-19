#include "soundmanager.h"

#include <glow/logging.h>

#include <fmod.hpp>
#include <fmod_errors.h>

SoundManager * SoundManager::s_instance = nullptr;

SoundManager* SoundManager::instance()
{
    assert(s_instance);
    return s_instance;

}

void SoundManager::initialize()
{
    assert(s_instance == nullptr);
    s_instance = new SoundManager();
}

void SoundManager::release()
{
    assert(s_instance);
    delete s_instance;
    s_instance = nullptr;
}

SoundManager::SoundManager()
{
    _result = FMOD::System_Create(&_system);
    ERRCHECK(_result);

    _result = _system->getVersion(&_version);
    ERRCHECK(_result);

    if (_version < FMOD_VERSION){
        glow::fatal("You are using an old version of FMOD %08x;.  This program requires %08x;\n", _version, FMOD_VERSION);
        exit(-1);
    }

    _result = _system->getNumDrivers(reinterpret_cast<int*>(&_numdrivers));
    ERRCHECK(_result);

    if (_numdrivers == 0){
        _result = _system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(_result);
    }else{
        _result = _system->getDriverCaps(0, &_caps, 0, &_speakermode);
        ERRCHECK(_result);

        // set the user selected speaker mode.
        _result = _system->setSpeakerMode(_speakermode);
        ERRCHECK(_result);

        // if the user has the 'Acceleration' slider set to off!  (bad for latency)
        if (_caps & FMOD_CAPS_HARDWARE_EMULATED){
            // maybe print pout a warning?
            _result = _system->setDSPBufferSize(1024, 10);
            ERRCHECK(_result);
        }

        _result = _system->getDriverInfo(0, _name, 256, 0);
        ERRCHECK(_result);

        // Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it.
        if (strstr(_name, "SigmaTel"))
        {
            _result = _system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(_result);
        }
    }

    // init _system with maximum of 100 channels
    _result = _system->init(100, FMOD_INIT_NORMAL, 0);
    // if the speaker mode selected isn't supported by this soundcard, switch it back to stereo
    if (_result == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        _result = _system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(_result);

        _result = _system->init(100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        ERRCHECK(_result);
    }

    _result = _system->set3DSettings(1.0, _distanceFactor, 1.0f);
    ERRCHECK(_result);

    FMOD_VECTOR f_position = { 0, 0, 0 };
    FMOD_VECTOR f_velocity = { 0, 0, 0 };
    FMOD_VECTOR f_forward = { 0, 0, 0 };
    FMOD_VECTOR f_up = { 0, 1, 0 };

    _result = _system->set3DListenerAttributes(0, &f_position, &f_velocity, &f_forward, &f_up);
    ERRCHECK(_result);
}

SoundManager::~SoundManager()
{
    for (SoundMap::iterator it = _channels.begin(); it != _channels.end(); ++it){
        _result = it->second.sound->release();
        ERRCHECK(_result);
    }

    _result = _system->close();
    ERRCHECK(_result);
    _result = _system->release();
    ERRCHECK(_result);
}

void SoundManager::ERRCHECK(FMOD_RESULT _result)
{
    if (_result != FMOD_OK)
    {
        glow::warning("FMOD error! %; (%;)", FMOD_ErrorString(_result), _result);
    }
}

unsigned int SoundManager::getNextFreeId()
{
    unsigned int i = 0;
    while (i < _channels.size()){
        if (_channels.find(i) == _channels.end())
            return i;
        ++i;
    }
    return i;
}

unsigned int SoundManager::createNewChannel(const std::string & soundFilePath, bool isLoop, bool is3D, bool paused, const glm::vec3& pos, const glm::vec3& vel)
{
    FMOD::Sound *_sound;
    FMOD::Channel *_channel = 0;
    unsigned int _id = getNextFreeId();
    FMOD_MODE _mode;

    if (is3D){
        _mode = FMOD_3D;
    }else{
        _mode = FMOD_SOFTWARE | FMOD_2D;
    }

    // create the sound

    _result = _system->createSound(soundFilePath.c_str(), _mode, 0, &_sound);
    ERRCHECK(_result);

    // set sound properties
    if (is3D){
        _result = _sound->set3DMinMaxDistance(0.5f * _distanceFactor, 5000.0f * _distanceFactor);
        ERRCHECK(_result);
    }

    if (isLoop){
        _result = _sound->setMode(FMOD_LOOP_NORMAL);
        ERRCHECK(_result);
    }

    // create the channel
    _result = _system->playSound(FMOD_CHANNEL_FREE, _sound, true, &_channel);
    ERRCHECK(_result);

    // set channel properties
    FMOD_VECTOR f_pos = { pos.x, pos.y, pos.z };
    FMOD_VECTOR f_vel = { vel.x, vel.y, vel.z };

    if (is3D){
        _result = _channel->set3DAttributes(&f_pos, &f_vel);
        ERRCHECK(_result);
    }

    if (!paused){
        _result = _channel->setPaused(false);
        ERRCHECK(_result);
    }

    _channels[_id] = { isLoop, is3D, f_pos, f_vel, _channel, _sound };
    _system->update();
    return _id;
}

void SoundManager::deleteChannel(unsigned int channelId)
{
    _channels[channelId].sound->release();
    ERRCHECK(_result);
    _channels.erase(channelId);
}

void SoundManager::play(unsigned int channelId)
{
    bool _play;
    bool _paused;
    _channels[channelId].channel->isPlaying(&_play);
    _channels[channelId].channel->getPaused(&_paused);
    if (_play && _paused){
        _result = _channels[channelId].channel->setPaused(false);
    }else{
        _result = _channels[channelId].channel->stop();
        ERRCHECK(_result);
        if (_channels[channelId].is3D){
            _result = _system->playSound(FMOD_CHANNEL_FREE, _channels[channelId].sound, true, &(_channels[channelId].channel));
            ERRCHECK(_result);
            _result = _channels[channelId].channel->set3DAttributes(&_channels[channelId].position, &_channels[channelId].velocity);
            ERRCHECK(_result);
            _result = _system->update();
            ERRCHECK(_result);
        _result = _channels[channelId].channel->setPaused(false);
        }else{
            _result = _system->playSound(FMOD_CHANNEL_FREE, _channels[channelId].sound, false, &(_channels[channelId].channel));
        }
    }
    ERRCHECK(_result);
}

void SoundManager::setPaused(unsigned int channelId, bool paused)
{
    _channels[channelId].channel->setPaused(paused);
}

bool SoundManager::isPaused(unsigned int channelId)
{
    bool p;
    _channels[channelId].channel->getPaused(&p);
    return p;
}

void SoundManager::togglePause(unsigned int channelId)
{
    bool p;
    _channels[channelId].channel->getPaused(&p);
    _channels[channelId].channel->setPaused(!p);
}

void SoundManager::setListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& velocity)
{
    FMOD_VECTOR f_pos = { pos.x, pos.y, pos.z };
    FMOD_VECTOR f_velocity = { velocity.x, velocity.y, velocity.z };
    FMOD_VECTOR f_forward = { forward.x, forward.y, forward.z };
    FMOD_VECTOR f_up = { up.x, up.y, up.z };

    _result = _system->set3DListenerAttributes(0, &f_pos, &f_velocity, &f_forward, &f_up);
    ERRCHECK(_result);
}

void SoundManager::setSoundPos(unsigned int channelId, const glm::vec3& pos)
{
    FMOD_VECTOR _vel = _channels[channelId].velocity;
    FMOD_VECTOR f_pos = { pos.x, pos.y, pos.z };    

    bool p;
    _channels[channelId].channel->isPlaying(&p);
    if (p){
        _result = _channels[channelId].channel->set3DAttributes(&f_pos, &_vel);
    }
    ERRCHECK(_result);
    _channels[channelId].position = f_pos;
}

void SoundManager::setSoundVel(unsigned int channelId, const glm::vec3& vel)
{
    FMOD_VECTOR _pos = _channels[channelId].position;
    FMOD_VECTOR f_vel = { vel.x, vel.y, vel.z };

    bool p;
    _channels[channelId].channel->isPlaying(&p);
    if (p){
        _result = _channels[channelId].channel->set3DAttributes(&_pos, &f_vel);
    }
    ERRCHECK(_result);
    _channels[channelId].velocity = f_vel;
}

void SoundManager::setSoundPosAndVel(unsigned int channelId, const glm::vec3& pos, const glm::vec3& vel)
{
    bool p;
    FMOD_VECTOR f_pos = { pos.x, pos.y, pos.z };
    FMOD_VECTOR f_vel = { vel.x, vel.y, vel.z };

    _channels[channelId].channel->isPlaying(&p);
    if (p){
        _result = _channels[channelId].channel->set3DAttributes(&f_pos, &f_vel);
    }
    _channels[channelId].position = f_pos;
    _channels[channelId].velocity = f_vel;
}

void SoundManager::setMute(unsigned int channelId, bool mute)
{
    _channels[channelId].channel->setMute(mute);
}

bool SoundManager::isMute(unsigned int channelId){
    bool mute;
    _channels[channelId].channel->getMute(&mute);
    return mute;
}

void SoundManager::setVolume(unsigned int channelId, float vol)
{
    _channels[channelId].channel->setVolume(vol);
}

float SoundManager::getVolume(unsigned int channelId)
{
    float vol;
    _channels[channelId].channel->getVolume(&vol);
    return vol;
}

void SoundManager::changeVolume(unsigned int channelId, float dVol)
{
    _channels[channelId].channel->setVolume(getVolume(channelId) + dVol);
}

void SoundManager::changeFile(unsigned int channelId, const std::string & filePath)
{
    FMOD_MODE _mode;
    bool _ispaused;
    // get current sound properties
    _channels[channelId].channel->getPaused(&_ispaused);
    _channels[channelId].sound->getMode(&_mode);
    // stop and release current sound object
    _channels[channelId].channel->stop();
    _channels[channelId].sound->release();

    // create new sound object
    _result = _system->createSound(filePath.c_str(), _mode, 0, &(_channels[channelId].sound));
    ERRCHECK(_result);
    // bind it to the channel
    _result = _system->playSound(FMOD_CHANNEL_REUSE, _channels[channelId].sound, _ispaused, &(_channels[channelId].channel));
    ERRCHECK(_result);
}

void SoundManager::update()
{
    _system->update();
}
