// SoundSystem.h
#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <iostream>
#include <thread>

#pragma comment(lib, "winmm.lib")

namespace game {

    class SoundSystem {
    public:
        enum Sound {
            CHEESE_COLLECT,
            POWERUP_PICKUP,
            LOSE_LIFE,
            LEVEL_COMPLETE,
            GAME_WIN,
            GAME_LOSE,
            BACKGROUND_MUSIC,
            CAT_CHASE,
            SHIELD_ACTIVE,
            SPEED_BOOST,
            FREEZE_EFFECT,
            MENU_MUSIC
        };

        SoundSystem() : musicPlaying_(false), musicThread_(nullptr) {
            std::cout << "  Initializing Sound System...\n";
        }

        ~SoundSystem() {
            StopMusic();
        }

        void Init() {
            std::cout << "  Sound System initialized\n";
        }

        void Play(Sound sound) {
            switch (sound) {
            case CHEESE_COLLECT:
                // Bouncy pickup
                PlayBeep(1046, 70);
                PlayBeep(1318, 70);
                PlayBeep(1568, 120);
                break;

            case POWERUP_PICKUP:
                // Magical sparkle
                PlayBeep(1318, 50);
                PlayBeep(1568, 50);
                PlayBeep(2093, 50);
                PlayBeep(2637, 150);
                break;

            case LOSE_LIFE:
                // Cartoon "bonk"
                PlayBeep(880, 100);
                PlayBeep(440, 100);
                PlayBeep(220, 200);
                break;

            case LEVEL_COMPLETE:
                // Ta-da!
                PlayBeep(659, 100);
                PlayBeep(784, 100);
                PlayBeep(880, 100);
                PlayBeep(1046, 200);
                PlayBeep(1318, 300);
                break;

            case GAME_WIN:
                // Celebration
                PlayBeep(1046, 100);
                PlayBeep(1318, 100);
                PlayBeep(1568, 100);
                PlayBeep(2093, 150);
                Sleep(80);
                PlayBeep(2093, 100);
                PlayBeep(2637, 400);
                break;

            case GAME_LOSE:
                // Wah wah wahhh
                PlayBeep(659, 200);
                PlayBeep(622, 200);
                PlayBeep(587, 200);
                PlayBeep(523, 400);
                break;

            case CAT_CHASE:
                // Panic sound
                PlayBeep(1568, 50);
                PlayBeep(1318, 50);
                PlayBeep(1568, 50);
                break;

            case SHIELD_ACTIVE:
                // Shield up!
                PlayBeep(784, 60);
                PlayBeep(1046, 60);
                PlayBeep(1318, 60);
                PlayBeep(1568, 120);
                break;

            case SPEED_BOOST:
                // Zoom!
                PlayBeep(523, 30);
                PlayBeep(659, 30);
                PlayBeep(784, 30);
                PlayBeep(1046, 30);
                PlayBeep(1318, 30);
                PlayBeep(1568, 80);
                break;

            case FREEZE_EFFECT:
                // Crystalline freeze
                PlayBeep(2093, 80);
                PlayBeep(1568, 80);
                PlayBeep(1318, 80);
                PlayBeep(1046, 150);
                break;
            }
        }

        void PlayBackgroundMusic() {
            if (!musicPlaying_) {
                musicPlaying_ = true;
                std::thread([this]() {
                    while (musicPlaying_) {
                        // Upbeat chase theme
                        PlayBeep(523, 150);
                        PlayBeep(659, 150);
                        PlayBeep(784, 150);
                        PlayBeep(659, 150);
                        PlayBeep(523, 150);
                        PlayBeep(659, 150);
                        PlayBeep(784, 300);

                        if (!musicPlaying_) break;

                        PlayBeep(880, 150);
                        PlayBeep(1046, 150);
                        PlayBeep(880, 150);
                        PlayBeep(784, 150);
                        PlayBeep(659, 300);

                        if (!musicPlaying_) break;
                        Sleep(200);
                    }
                    }).detach();
            }
        }

        void StopMusic() {
            musicPlaying_ = false;
        }

    private:
        bool musicPlaying_;
        std::thread* musicThread_;

        void PlayBeep(int frequency, int duration) {
            Beep(frequency, duration);
        }
    };

} // namespace game