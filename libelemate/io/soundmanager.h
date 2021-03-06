#pragma once

#include <map>
#include <unordered_map>

#include <fmod.h>

#include <glm/glm.hpp>

namespace FMOD {
    class Channel;
    class Sound;
    class System;
}

class SoundManager{
public:
    static void initialize();
    static void release();
    static SoundManager* instance();

    /** creates a new channel and returns the channelId */
    unsigned int createNewChannel(const std::string & soundFilePath, bool isLoop, bool is3D, bool paused = false, const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3& vel = glm::vec3(0.0f, 0.0f, 0.0f));
    /** deletes a channel */
    void deleteChannel(unsigned int channelId);
    /** unpauses a loop-channel; unpauses a paused non-loop-channel, otherwise plays it from the beginning */
    void play(unsigned int channelId);
    /** pauses/unpauses a channel */
    void setPaused(unsigned int channelId, bool paused);
    /** returns true if the channel is paused */
    bool isPaused(unsigned int channelId);
    /** toggles the pause status of channel */
    void togglePause(unsigned int channelId);
    /** sets general Listener attributes */
    void setListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up, const glm::vec3& velocity = glm::vec3( 0.f, 0.f, 0.f));
    /** sets position of a channel in the world */
    void setSoundPosition(unsigned int channelId, const glm::vec3& pos);
    /** sets the velocity of a channel */
    void setSoundVelocity(unsigned int channelId, const glm::vec3& vel);
    /** sets positions and velocity of a channel in the world */
    void setSoundPositionAndVelocity(unsigned int channelId, const glm::vec3& pos, const glm::vec3& vel);
    /** mutes or unmutes the channel */
    void setMute(unsigned int channelId, bool mute);
    /** returns true if the channel is mute */
    bool isMute(unsigned int channelId);
    /** sets volume of a channel*/
    void setVolume(unsigned int channelId, float vol);
    /** returns the current volume of a channel*/
    float getVolume(unsigned int channelId);
    /** changes volume of a specific channel by dVol (should be between -1.f and 1.f) */
    void changeVolume(unsigned int channelId, float dVol);
    /** changes the played file of a channel */
    void changeFile(unsigned int channelId, const std::string & filePath);
    /** updates 3D positions */
    void update();
    
private:
    SoundManager();
    ~SoundManager();

    struct SoundObject{
        bool isLoop;
        bool is3D;
        FMOD_VECTOR     position;
        FMOD_VECTOR     velocity;
        FMOD::Channel   *channel;
        FMOD::Sound     *sound;
    };

    static SoundManager * s_instance;

    float                               m_distanceFactor = 5.f; // units per meter (centimeters = 100)
    std::map<unsigned int, SoundObject> m_channels;
    FMOD::System *                      m_system;

    /** checks errors */
    void ERRCHECK(FMOD_RESULT);
    /** returns the next free channelId */
    unsigned int getNextFreeId();

    FMOD::Sound * loadSound(const std::string & filename, bool isLoop, bool is3D);
    std::unordered_map<std::string, FMOD::Sound *> m_loadedSounds;

public:
    SoundManager(const SoundManager&) = delete;
    const SoundManager& operator=(const SoundManager&) = delete;
};
