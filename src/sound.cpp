#include <stdexcept>
#include <string>
#include <iostream>

#include <fangpp/sound.hpp>

Sound::Sound() : 
    system(init()),
    sfxMove("media/sfx_move.mp3", system, false),
    sfxInvalidMove("media/sfx_invalid_move.mp3", system, true),
    sfxCapture("media/sfx_capture.mp3", system, false),
    sfxTarget("media/sfx_target.mp3", system, false)
{
    // Load sounds into memory
    ERRCHECK(system->createSound("media/theme_player.ogg", 
                                 FMOD_LOOP_NORMAL, 
                                 nullptr, 
                                 &mainTheme));
    ERRCHECK(system->createSound("media/theme_boeg.ogg", 
                                 FMOD_LOOP_NORMAL, 
                                 nullptr, 
                                 &boegTheme));
}

FMOD::System *Sound::init()
{
    // Initialize sound system
    FMOD::System *system = nullptr;
    ERRCHECK(FMOD::System_Create(&system));
    ERRCHECK(system->init(maxChannels, FMOD_INIT_NORMAL, nullptr));
    
    return system;
}

void Sound::play(const Kind kind)
{
    // Begin playing sound on available channel
    switch (kind) 
    {
        case MAIN_THEME:
            if (isPlaying(mainChannel))
            {
                // Currently playing boeg theme: Stop playing and save position in theme
                ERRCHECK(mainChannel->getPosition(&boegThemePosPCM, FMOD_TIMEUNIT_PCM));
                ERRCHECK(mainChannel->stop());
            }
            ERRCHECK(system->playSound(mainTheme, nullptr, false, &mainChannel));
            // Restore position of main theme from last time
            ERRCHECK(mainChannel->setPosition(mainThemePosPCM, FMOD_TIMEUNIT_PCM));                
            break;
        case BOEG_THEME:
            if (isPlaying(mainChannel))
            {
                // Currently playing main theme: Stop playing and save position in theme
                ERRCHECK(mainChannel->getPosition(&mainThemePosPCM, FMOD_TIMEUNIT_PCM));
                ERRCHECK(mainChannel->stop());
            }
            
            ERRCHECK(system->playSound(boegTheme, nullptr, false, &mainChannel));
            // Restore position of boeg theme from last time
            ERRCHECK(mainChannel->setPosition(boegThemePosPCM, FMOD_TIMEUNIT_PCM));
            break;
        case SFX_MOVE:
            sfxMove.play(system, &sfxChannel);
            break;
        case SFX_INVALID_MOVE:
            sfxInvalidMove.play(system, &sfxChannel);
            break;
        case SFX_CAPTURE:
            sfxCapture.play(system, &sfxChannel);
            break;
        case SFX_TARGET:
            sfxTarget.play(system, &sfxChannel);
            break;
        default:
            throw std::runtime_error("Unrecognized kind of sound");
    }
}

bool Sound::isPlaying(FMOD::Channel *channel) const
{
    bool isPlaying = false;
    
    if (channel)
    {
        FATALERROR(channel->isPlaying(&isPlaying));
    }
    return isPlaying;
}

void Sound::update()
{
    ERRCHECK(system->update());
}

Sound::~Sound()
{
    // Cleanup sound system resources
    ERRCHECK(mainTheme->release());
    ERRCHECK(boegTheme->release());
    ERRCHECK(system->release());  // automatically calls close
}

void Sound::ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        throw std::runtime_error("FMOD Error: " + std::string(FMOD_ErrorString(result)));
    }
}

void Sound::FATALERROR(FMOD_RESULT result)
{
    if (result != FMOD_OK && 
        result != FMOD_ERR_CHANNEL_STOLEN &&
        result != FMOD_ERR_INVALID_HANDLE)
    {
        ERRCHECK(result);
    }
}
