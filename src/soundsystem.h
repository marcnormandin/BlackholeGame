#ifndef BlackHole_soundsystem_h
#define BlackHole_soundsystem_h

#include <stdexcept>

#include "SDL2_Mixer/SDL_mixer.h"

class SoundSystem {
public:
    SoundSystem() {
        // Sounds
        //Initialize SDL_mixer
        if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
        {
            throw std::runtime_error("Unable to load OpenAudio.");
        }
        
        
        /*
         Primitive background music commented out to save on music downloads.
         
        mMusic = Mix_LoadMUS( "../data/music.wav" );
        
        //If there was a problem loading the music
        if( mMusic == NULL )
        {
            throw std::runtime_error("Unable to load music.wave");
        }
        
        Mix_PlayMusic( mMusic, -1 );
        */
        
        // effects
        mFire = Mix_LoadWAV( "../data/fire.wav" );
        mExplosion = Mix_LoadWAV("../data/explosion.wav");
    }
    
    ~SoundSystem() {
        //Free the music
        //Mix_FreeMusic( mMusic );
        Mix_FreeChunk( mFire );
        Mix_FreeChunk( mExplosion );
        
        Mix_CloseAudio();
    }
    
    void playFire() {
        Mix_PlayChannel( -1, mFire, 0 );
    }
    
    void playExplosion() {
        Mix_PlayChannel( -1, mExplosion, 0 );
    }
    
private:
    //Mix_Music* mMusic{nullptr};
    Mix_Chunk* mFire{nullptr};
    Mix_Chunk* mExplosion{nullptr};
};

#endif
