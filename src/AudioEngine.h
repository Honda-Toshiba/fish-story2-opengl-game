#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <string>
#include <map>
#include <vector>

// Forward declare the engine struct to keep header clean
struct ma_engine;
struct ma_sound;

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool Initialize();
    void Deinitialize();

    // Load a sound from file and give it a name (e.g., "bubbles")
    void LoadSound(const std::string& name, const std::string& filePath, bool loop = false);
    
    // Play a specific sound
    void Play(const std::string& name);
    
    // Stop a sound (useful for stopping loops)
    void Stop(const std::string& name);
    
    // Check if a sound is currently playing
    bool IsPlaying(const std::string& name);

    void SetVolume(const std::string& name, float volume);
    
private:
    ma_engine* engine;
    std::map<std::string, ma_sound*> sounds;
};

#endif