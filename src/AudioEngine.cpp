#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioEngine.h"
#include <iostream>

AudioEngine::AudioEngine() : engine(nullptr) {}

AudioEngine::~AudioEngine() {
    Deinitialize();
}

bool AudioEngine::Initialize() {
    engine = new ma_engine;
    ma_result result = ma_engine_init(NULL, engine);
    
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine." << std::endl;
        delete engine;
        engine = nullptr;
        return false;
    }
    return true;
}

void AudioEngine::Deinitialize() {
    if (engine) {
        // Uninitialize all loaded sounds
        for (auto& pair : sounds) {
            ma_sound_uninit(pair.second);
            delete pair.second;
        }
        sounds.clear();
        
        ma_engine_uninit(engine);
        delete engine;
        engine = nullptr;
    }
}

void AudioEngine::SetVolume(const std::string& name, float volume) {
    if (sounds.find(name) != sounds.end()) {
        ma_sound_set_volume(sounds[name], volume);
    }
}

void AudioEngine::LoadSound(const std::string& name, const std::string& filePath, bool loop) {
    if (!engine) return;

    ma_sound* sound = new ma_sound;
    
    // Decode ensures the whole file is loaded into memory (good for small SFX)
    // Use MA_SOUND_FLAG_STREAM for large music files if needed
    ma_uint32 flags = MA_SOUND_FLAG_DECODE;
    
    ma_result result = ma_sound_init_from_file(engine, filePath.c_str(), flags, NULL, NULL, sound);
    
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load sound: " << filePath << std::endl;
        delete sound;
        return;
    }
    
    if (loop) {
        ma_sound_set_looping(sound, MA_TRUE);
    }
    
    sounds[name] = sound;
    std::cout << "Loaded sound: " << name << std::endl;
}

void AudioEngine::Play(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        // If it's already playing and not looping, rewind it to restart
        // If it IS looping, just ensure it's started
        ma_sound_start(sounds[name]);
    }
}

void AudioEngine::Stop(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        ma_sound_stop(sounds[name]);
        ma_sound_seek_to_pcm_frame(sounds[name], 0); // Reset to start
    }
}

bool AudioEngine::IsPlaying(const std::string& name) {
    if (sounds.find(name) != sounds.end()) {
        return ma_sound_is_playing(sounds[name]);
    }
    return false;
}