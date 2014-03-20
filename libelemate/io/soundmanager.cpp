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
    FMOD_RESULT result;
    result = FMOD::System_Create(&m_system);
    ERRCHECK(result);

    unsigned int version = 0;
    result = m_system->getVersion(&version);
    ERRCHECK(result);

    if (version < FMOD_VERSION){
        glow::fatal("You are using an old version of FMOD %08x;.  This program requires %08x;\n", version, FMOD_VERSION);
        exit(-1);
    }

    int numDrivers = 0;
    result = m_system->getNumDrivers(&numDrivers);
    ERRCHECK(result);

    if (numDrivers == 0){
        result = m_system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
        ERRCHECK(result);
    }else{
        FMOD_SPEAKERMODE speakerMode;
        FMOD_CAPS caps;
        result = m_system->getDriverCaps(0, &caps, 0, &speakerMode);
        ERRCHECK(result);

        // set the user selected speaker mode.
        result = m_system->setSpeakerMode(speakerMode);
        ERRCHECK(result);

        // if the user has the 'Acceleration' slider set to off!  (bad for latency)
        if (caps & FMOD_CAPS_HARDWARE_EMULATED){
            // maybe print pout a warning?
            result = m_system->setDSPBufferSize(1024, 10);
            ERRCHECK(result);
        }

        char name[256];
        result = m_system->getDriverInfo(0, name, 256, 0);
        ERRCHECK(result);

        // Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it.
        if (strstr(name, "SigmaTel"))
        {
            result = m_system->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0, 0, FMOD_DSP_RESAMPLER_LINEAR);
            ERRCHECK(result);
        }
    }

    // init _system with maximum of 100 channels
    result = m_system->init(100, FMOD_INIT_NORMAL, 0);
    // if the speaker mode selected isn't supported by this soundcard, switch it back to stereo
    if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        result = m_system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        ERRCHECK(result);

        result = m_system->init(100, FMOD_INIT_NORMAL, 0);/* ... and re-init. */
        ERRCHECK(result);
    }

    result = m_system->set3DSettings(1.0, m_distanceFactor, 1.0f);
    ERRCHECK(result);

    FMOD_VECTOR f_position = { 0, 0, 0 };
    FMOD_VECTOR f_velocity = { 0, 0, 0 };
    FMOD_VECTOR f_forward = { 0, 0, 0 };
    FMOD_VECTOR f_up = { 0, 1, 0 };

    result = m_system->set3DListenerAttributes(0, &f_position, &f_velocity, &f_forward, &f_up);
    ERRCHECK(result);
}

SoundManager::~SoundManager()
{
    FMOD_RESULT result;
    for (auto & it : m_loadedSounds) {
        result = it.second->release();
        ERRCHECK(result);
    }

    result = m_system->close();
    ERRCHECK(result);
    result = m_system->release();
    ERRCHECK(result);
}

void SoundManager::ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        glow::warning("FMOD error! %; (%;)", FMOD_ErrorString(result), result);
    }
    assert(result == FMOD_OK);
}

unsigned int SoundManager::getNextFreeId()
{
    return static_cast<unsigned int>(m_channels.size());
}

FMOD::Sound * SoundManager::loadSound(const std::string & fileName, bool isLoop, bool is3D)
{
    // reuse already loaded sounds
    const auto it = m_loadedSounds.find(fileName);
    if (it != m_loadedSounds.end()) {
        return it->second;
    }

    // or load the sound if needed
    FMOD::Sound * sound = nullptr;
    FMOD_MODE mode;
    if (is3D) {
        mode = FMOD_3D;
    }
    else {
        mode = FMOD_SOFTWARE | FMOD_2D;
    }

    // create the sound
    FMOD_RESULT result;
    result = m_system->createSound(fileName.c_str(), mode, 0, &sound);
    ERRCHECK(result);

    // set sound properties
    if (is3D) {
        result = sound->set3DMinMaxDistance(0.5f * m_distanceFactor, 5000.0f * m_distanceFactor);
        ERRCHECK(result);
    }

    if (isLoop) {
        result = sound->setMode(FMOD_LOOP_NORMAL);
        ERRCHECK(result);
    }

    m_loadedSounds.emplace(fileName, sound);

    return sound;
}

unsigned int SoundManager::createNewChannel(const std::string & soundFilePath, bool isLoop, bool is3D, bool paused, const glm::vec3& pos, const glm::vec3& vel)
{
    FMOD::Sound * sound = nullptr;
    FMOD::Channel * channel = nullptr;
    unsigned int id = getNextFreeId();

    // try to load the sound
    sound = loadSound(soundFilePath, isLoop, is3D);
    if (sound == nullptr) {
        return 0;
    }


    // create the channel
    FMOD_RESULT result = m_system->playSound(FMOD_CHANNEL_FREE, sound, true, &channel);
    ERRCHECK(result);

    // set channel properties
    FMOD_VECTOR position = { pos.x, pos.y, pos.z };
    FMOD_VECTOR velocity = { vel.x, vel.y, vel.z };

    if (is3D) {
        result = channel->set3DAttributes(&position, &velocity);
        ERRCHECK(result);
    }

    if (!paused) {
        result = channel->setPaused(false);
        ERRCHECK(result);
    }

    m_channels[id] = { isLoop, is3D, position, velocity, channel, sound };
    m_system->update();
    return id;
}

void SoundManager::deleteChannel(unsigned int channelId)
{
    m_channels.erase(channelId);
}

void SoundManager::play(unsigned int channelId)
{
    bool play;
    bool paused;
    FMOD_RESULT result;
    m_channels[channelId].channel->isPlaying(&play);
    m_channels[channelId].channel->getPaused(&paused);
    if (play && paused) {
        result = m_channels[channelId].channel->setPaused(false);
    } else {
        result = m_channels[channelId].channel->stop();
        ERRCHECK(result);
        if (m_channels[channelId].is3D){
            result = m_system->playSound(FMOD_CHANNEL_FREE, m_channels[channelId].sound, true, &(m_channels[channelId].channel));
            ERRCHECK(result);
            result = m_channels[channelId].channel->set3DAttributes(&m_channels[channelId].position, &m_channels[channelId].velocity);
            ERRCHECK(result);
            result = m_system->update();
            ERRCHECK(result);
        result = m_channels[channelId].channel->setPaused(false);
        } else {
            result = m_system->playSound(FMOD_CHANNEL_FREE, m_channels[channelId].sound, false, &(m_channels[channelId].channel));
        }
    }
    ERRCHECK(result);
}

void SoundManager::setPaused(unsigned int channelId, bool paused)
{
    m_channels[channelId].channel->setPaused(paused);
}

bool SoundManager::isPaused(unsigned int channelId)
{
    bool p;
    m_channels[channelId].channel->getPaused(&p);
    return p;
}

void SoundManager::togglePause(unsigned int channelId)
{
    bool p;
    m_channels[channelId].channel->getPaused(&p);
    m_channels[channelId].channel->setPaused(!p);
}

void SoundManager::setListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& velocity)
{
    FMOD_VECTOR f_pos = { pos.x, pos.y, pos.z };
    FMOD_VECTOR f_velocity = { velocity.x, velocity.y, velocity.z };
    FMOD_VECTOR f_forward = { forward.x, forward.y, forward.z };
    FMOD_VECTOR f_up = { up.x, up.y, up.z };

    FMOD_RESULT result = m_system->set3DListenerAttributes(0, &f_pos, &f_velocity, &f_forward, &f_up);
    ERRCHECK(result);
}

void SoundManager::setSoundPosition(unsigned int channelId, const glm::vec3& pos)
{
    FMOD_VECTOR velocity = m_channels[channelId].velocity;
    FMOD_VECTOR position = { pos.x, pos.y, pos.z };    

    bool p;
    m_channels[channelId].channel->isPlaying(&p);
    if (p) {
        FMOD_RESULT result = m_channels[channelId].channel->set3DAttributes(&position, &velocity);
        ERRCHECK(result);
    }
    m_channels[channelId].position = position;
}

void SoundManager::setSoundVelocity(unsigned int channelId, const glm::vec3& vel)
{
    FMOD_VECTOR _pos = m_channels[channelId].position;
    FMOD_VECTOR f_vel = { vel.x, vel.y, vel.z };

    bool p;
    m_channels[channelId].channel->isPlaying(&p);
    if (p) {
        FMOD_RESULT result = m_channels[channelId].channel->set3DAttributes(&_pos, &f_vel);
        ERRCHECK(result);
    }
    m_channels[channelId].velocity = f_vel;
}

void SoundManager::setSoundPositionAndVelocity(unsigned int channelId, const glm::vec3& pos, const glm::vec3& vel)
{
    bool p;
    FMOD_VECTOR position = { pos.x, pos.y, pos.z };
    FMOD_VECTOR velocity = { vel.x, vel.y, vel.z };

    m_channels[channelId].channel->isPlaying(&p);
    if (p) {
        FMOD_RESULT result = m_channels[channelId].channel->set3DAttributes(&position, &velocity);
        ERRCHECK(result);
    }
    m_channels[channelId].position = position;
    m_channels[channelId].velocity = velocity;
}

void SoundManager::setMute(unsigned int channelId, bool mute)
{
    m_channels[channelId].channel->setMute(mute);
}

bool SoundManager::isMute(unsigned int channelId){
    bool mute;
    m_channels[channelId].channel->getMute(&mute);
    return mute;
}

void SoundManager::setVolume(unsigned int channelId, float vol)
{
    m_channels[channelId].channel->setVolume(vol);
}

float SoundManager::getVolume(unsigned int channelId)
{
    float vol;
    m_channels[channelId].channel->getVolume(&vol);
    return vol;
}

void SoundManager::changeVolume(unsigned int channelId, float dVol)
{
    m_channels[channelId].channel->setVolume(getVolume(channelId) + dVol);
}

void SoundManager::changeFile(unsigned int channelId, const std::string & filePath)
{
    FMOD_MODE mode;
    bool isPaused;
    // get current sound properties
    m_channels[channelId].channel->getPaused(&isPaused);
    m_channels[channelId].sound->getMode(&mode);
    // stop and release current sound object
    m_channels[channelId].channel->stop();
    m_channels[channelId].sound->release();

    // create new sound object
    FMOD_RESULT result = m_system->createSound(filePath.c_str(), mode, 0, &(m_channels[channelId].sound));
    ERRCHECK(result);
    // bind it to the channel
    result = m_system->playSound(FMOD_CHANNEL_REUSE, m_channels[channelId].sound, isPaused, &(m_channels[channelId].channel));
    ERRCHECK(result);
}

void SoundManager::update()
{
    m_system->update();
}
