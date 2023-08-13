#include <stdexcept>
#include <string>
#include <iostream>

#include <fangpp/sound.hpp>

Sound::Sound() : system(init()), click("media/sfx_select.ogg", system)
{
    // Load sounds into memory
    ERRCHECK(system->createSound("media/theme_player.ogg", 
                                 FMOD_LOOP_NORMAL, 
                                 nullptr, 
                                 &mainTheme));
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
            ERRCHECK(system->playSound(mainTheme, nullptr, false, &channel));
            break;
        case CLICK_SFX:
            click.play(system, channel);
            break;
        default:
            throw std::runtime_error("Unrecognized kind of sound");
    }
}

bool Sound::isPlaying() const
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
    ERRCHECK(click.sound->release());
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
