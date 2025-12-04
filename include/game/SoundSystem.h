// SoundSystem.h - Enhanced with REAL MP3 Playback Support
#pragma once
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <iostream>
#include <thread>
#include <atomic>
#include <memory>

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

        SoundSystem()
            : musicPlaying_(false)
            , musicPaused_(false)
            , stopRequested_(false)
            , musicThread_(nullptr)
            , useMp3_(false) {
            std::cout << "  Initializing Sound System...\n";
        }

        ~SoundSystem() {
            StopMusic();
            if (musicThread_ && musicThread_->joinable()) {
                musicThread_->join();
            }

            // Stop MCI playback if active
            if (useMp3_) {
                mciSendStringA("close TomJerryMusic", NULL, 0, NULL);
            }
        }

        void Init() {
            // Try to load MP3 file - FIXED: Look in audio folder, not sounds!
            std::string musicPath = "assets/audio/tom_and_jerry.mp3";

            std::cout << "  Checking for MP3 at: " << musicPath << "\n";

            // Check if file exists
            DWORD fileAttr = GetFileAttributesA(musicPath.c_str());
            if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
                std::cout << "  Found Tom & Jerry MP3 file!\n";
                useMp3_ = true;
                mp3Path_ = musicPath;
            }
            else {
                std::cout << "  MP3 not found at: " << musicPath << "\n";
                std::cout << "  Using beep music instead\n";
                useMp3_ = false;
            }

            std::cout << "  Sound System initialized\n";
        }

        void Play(Sound sound) {
            // Play sounds in a separate thread so they don't block gameplay
            std::thread([this, sound]() {
                this->PlaySoundEffect(sound);
                }).detach();
        }

        void PlayBackgroundMusic() {
            if (musicPlaying_) {
                std::cout << "Music already playing\n";
                return;
            }

            stopRequested_ = false;
            musicPlaying_ = true;
            musicPaused_ = false;

            if (useMp3_) {
                std::cout << "Starting Tom & Jerry MP3 music!\n";
                PlayMP3Music();
            }
            else {
                std::cout << "Starting beep chase music!\n";

                // Create new thread for beep music
                musicThread_ = std::make_unique<std::thread>([this]() {
                    this->BeepMusicLoop();
                    });
            }
        }

        void StopMusic() {
            if (!musicPlaying_) return;

            std::cout << "Stopping music...\n";
            stopRequested_ = true;
            musicPlaying_ = false;
            musicPaused_ = false;

            if (useMp3_) {
                mciSendStringA("stop TomJerryMusic", NULL, 0, NULL);
                mciSendStringA("close TomJerryMusic", NULL, 0, NULL);
            }
            else {
                if (musicThread_ && musicThread_->joinable()) {
                    musicThread_->join();
                    musicThread_.reset();
                }
            }
        }

        void PauseMusic() {
            if (musicPlaying_ && !musicPaused_) {
                musicPaused_ = true;
                std::cout << "Music paused\n";

                if (useMp3_) {
                    mciSendStringA("pause TomJerryMusic", NULL, 0, NULL);
                }
            }
        }

        void ResumeMusic() {
            if (musicPlaying_ && musicPaused_) {
                musicPaused_ = false;
                std::cout << "Music resumed\n";

                if (useMp3_) {
                    mciSendStringA("resume TomJerryMusic", NULL, 0, NULL);
                }
            }
        }

    private:
        std::atomic<bool> musicPlaying_;
        std::atomic<bool> musicPaused_;
        std::atomic<bool> stopRequested_;
        std::unique_ptr<std::thread> musicThread_;
        bool useMp3_;
        std::string mp3Path_;

        void PlayBeep(int frequency, int duration) {
            Beep(frequency, duration);
        }

        void PlayMP3Music() {
            // Use MCI (Media Control Interface) to play MP3
            std::cout << "  Attempting to play MP3: " << mp3Path_ << "\n";

            // Build the command string
            std::string openCmd = "open \"" + mp3Path_ + "\" type mpegvideo alias TomJerryMusic";
            std::cout << "  MCI Command: " << openCmd << "\n";

            // Open the MP3 file
            MCIERROR error = mciSendStringA(openCmd.c_str(), NULL, 0, NULL);
            if (error != 0) {
                char errorMsg[256];
                mciGetErrorStringA(error, errorMsg, 256);
                std::cerr << "  ERROR opening MP3: " << errorMsg << "\n";
                std::cerr << "  Error code: " << error << "\n";

                // Fallback to beep music
                std::cout << "  Falling back to beep music\n";
                useMp3_ = false;
                musicThread_ = std::make_unique<std::thread>([this]() {
                    this->BeepMusicLoop();
                    });
                return;
            }

            std::cout << "  MP3 opened successfully!\n";

            // Play the music with looping
            error = mciSendStringA("play TomJerryMusic repeat", NULL, 0, NULL);
            if (error != 0) {
                char errorMsg[256];
                mciGetErrorStringA(error, errorMsg, 256);
                std::cerr << "  ERROR playing MP3: " << errorMsg << "\n";
            }
            else {
                std::cout << "  MP3 music now playing!\n";
            }
        }

        void PlaySoundEffect(Sound sound) {
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

        void BeepMusicLoop() {
            // Enhanced Tom & Jerry chase theme - more energetic!
            struct Note {
                int freq;
                int duration;
            };

            // Main chase melody
            Note melody[] = {
                // Part 1: Fast chase theme
                {523, 120},   // C
                {659, 120},   // E
                {784, 120},   // G
                {1046, 120},  // C
                {784, 120},   // G
                {659, 120},   // E
                {523, 180},   // C

                // Part 2: Tension build
                {587, 120},   // D
                {740, 120},   // F#
                {880, 120},   // A
                {1174, 120},  // D
                {880, 120},   // A
                {740, 120},   // F#
                {587, 180},   // D

                // Part 3: Playful run
                {659, 100},   // E
                {784, 100},   // G
                {880, 100},   // A
                {1046, 100},  // C
                {1318, 100},  // E
                {1568, 200},  // G

                // Part 4: Cat pounce
                {1046, 80},   // C
                {880, 80},    // A
                {784, 80},    // G
                {659, 80},    // E
                {523, 80},    // C
                {392, 180},   // G low

                // Part 5: Mouse escape!
                {1318, 100},  // E high
                {1174, 100},  // D
                {1046, 100},  // C
                {880, 100},   // A
                {784, 200},   // G

                // Part 6: Victory lap
                {1046, 120},  // C
                {1318, 120},  // E
                {1568, 120},  // G
                {1046, 120},  // C
                {784, 240},   // G
            };

            int melodyLength = sizeof(melody) / sizeof(Note);

            while (musicPlaying_ && !stopRequested_) {
                // Wait if paused
                while (musicPaused_ && musicPlaying_ && !stopRequested_) {
                    Sleep(100);
                }

                if (stopRequested_ || !musicPlaying_) break;

                // Play the full melody
                for (int i = 0; i < melodyLength && musicPlaying_ && !stopRequested_; i++) {
                    // Check pause state
                    while (musicPaused_ && musicPlaying_ && !stopRequested_) {
                        Sleep(100);
                    }

                    if (stopRequested_ || !musicPlaying_) break;

                    PlayBeep(melody[i].freq, melody[i].duration);

                    // Small gap between notes for rhythm
                    Sleep(30);
                }

                // Brief pause before looping
                if (musicPlaying_ && !stopRequested_) {
                    Sleep(500);
                }
            }

            musicPlaying_ = false;
            std::cout << "Music stopped\n";
        }
    };

} // namespace game