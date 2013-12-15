# pragma once

#include <vector>
#include <string>
#include <map>

#include "fmod.hpp"

class SoundManager{
public:
    /** constructor */
    SoundManager(FMOD_VECTOR startPosition = { 0.f, 0.f, 0.f });
    /** destructor */
    ~SoundManager();
    /** creates a new channel and returns the channelId */
    int createNewChannel(std::string soundFilePath, bool isLoop, bool is3D, bool paused = false, FMOD_VECTOR pos = { 0.0f, 0.0f, 0.0f }, FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f });
    /** deletes a channel */
    void deleteChannel(int channelId);
    /** unpauses a loop-channel; unpauses a paused non-loop-channel, otherwise plays it from the beginning */
    void play(int channelId);
    /** pauses/unpauses a channel */
    void setPaused(int channelId, bool paused);
    /** returns true if the channel is paused */
    bool isPaused(int channelId);
    /** toggles the pause status of channel */
    void togglePause(int channelId);
    /** sets general Listener attributes */
    void setListenerAttributes(FMOD_VECTOR pos, FMOD_VECTOR forward, FMOD_VECTOR up, FMOD_VECTOR velocity = { 0.f, 0.f, 0.f });
    /** sets position of a channel in the world */
    void setSoundPos(int channelId, FMOD_VECTOR pos);
    /** sets the velocity of a channel */
    void setSoundVel(int channelId, FMOD_VECTOR vel);
    /** sets positions and velocity of a channel in the world */
    void setSoundPosAndVel(int channelId, FMOD_VECTOR pos, FMOD_VECTOR vel);
    /** mutes or unmutes the channel */
    void setMute(int channelId, bool mute);
    /** returns true if the channel is mute */
    bool isMute(int channelId);
    /** sets volume of a channel*/
    void setVolume(int channelId, float vol);
    /** returns the current volume of a channel*/
    float getVolume(int channelId);
    /** changes volume of a specific channel by dVol (should be between -1.f and 1.f) */
    void changeVolume(int channelId, float dVol);
    /** changes the played file of a channel */
    void changeFile(int channelId, std::string filePath);
    /** updates 3D positions */
    void update();
    
private:
    typedef struct SoundObject{
        bool isLoop;
        bool is3D;
        FMOD_VECTOR     position;
        FMOD_VECTOR     velocity;
        FMOD::Channel   *channel;
        FMOD::Sound     *sound;
    }SoundObject;
    typedef std::map<int, SoundObject> SoundMap;

    unsigned int        _version;
    int                 _numdrivers;
    FMOD_SPEAKERMODE    _speakermode;
    FMOD_CAPS           _caps;
    char                _name[256];
    float               _distanceFactor = 5.f; // units per meter (centimeters = 100)
    SoundMap            _channels;
    FMOD_RESULT         _result;
    FMOD::System        *_system;
    /** checks errors */
    void SoundManager::ERRCHECK(FMOD_RESULT);
    /** returns the next free channelId */
    int getNextFreeId();
    /** inits the SoundManager with optional start position */
    void init(FMOD_VECTOR position = { 0.f, 0.f, 0.f }, FMOD_VECTOR forward = { 0.f, 0.f, 1.f }, FMOD_VECTOR up = { 0.f, 1.f, 0.f }, FMOD_VECTOR velocity = { 0.f, 0.f, 0.f });
};
