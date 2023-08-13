#ifndef SOUND_HPP
#define SOUND_HPP

#include "fmod.hpp"
#include "fmod_errors.h"

#include <chrono>
#include <optional>

class Sound
{
public:
    enum Kind {
        MAIN_THEME = 0,
        CLICK_SFX
    };
    
    Sound();
    
    FMOD::System *init();
    
    void play(const Kind kind);
    
    bool isPlaying() const;
    
    void update();
    
    ~Sound();
    
    class CooldownTimer
    {
        public:
            using clock = std::chrono::steady_clock;
            using time_point = std::chrono::time_point<clock>;
            
            CooldownTimer(double cooldown = 0.0) : m_cooldown(cooldown) {}
            
            bool hasExpired()
            {
                // For the first time, there is no cooldown
                if (!m_start.has_value() && !m_end.has_value())
                {
                    m_start = clock::now();
                    return true;
                }
                
                // Measure current time point and elapsed time since cooldown start
                m_end = clock::now();
                const double elapsed = std::chrono::duration<double>(
                    m_end.value() - m_start.value()
                ).count();
                
                if (elapsed >= m_cooldown)
                {
                    // Restart timer
                    m_start = clock::now();
                    return true;
                }
                
                return false;
            }
            
            void setCooldown(const double cooldown) { m_cooldown = cooldown; }
            
        private:
            double m_cooldown;  // cooldown time in seconds
            std::optional<time_point> m_start;
            std::optional<time_point> m_end;
    };
    
    struct SoundEffect
    {
        SoundEffect(const char *path, FMOD::System *system)
        {
            ERRCHECK(system->createSound(path,
                                         FMOD_DEFAULT,
                                         nullptr,
                                         &sound));
    
            unsigned int lenMS = 0;
            FATALERROR(sound->getLength(&lenMS, FMOD_TIMEUNIT_MS));
            timer.setCooldown(static_cast<double>(lenMS) / 1000.0);
        }
        
        void play(FMOD::System *system, FMOD::Channel *channel)
        {
            if (timer.hasExpired())
                ERRCHECK(system->playSound(sound, nullptr, false, &channel));
        }
        
        FMOD::Sound *sound = nullptr;
        CooldownTimer timer;
    };
private:
    static constexpr int maxChannels = 32;
    
    static void ERRCHECK(FMOD_RESULT result);
    static void FATALERROR(FMOD_RESULT result);
    
    FMOD::System *system = nullptr;
    FMOD::Sound *mainTheme = nullptr;
    FMOD::Channel *channel = nullptr;
    SoundEffect click;
};

#endif /* SOUND_HPP */
