#ifndef BlackHole_sprite_h
#define BlackHole_sprite_h

#include <SDL2/SDL.h>

#include "spritesheet.h"
#include <memory>

// This is a handle to a subimage of a sprite sheet
class Sprite {
public:
    friend class SpriteSheet;
//private:
    Sprite (const SpriteSheet& sheet, const SDL_Rect& subimageRect)
    : mSpriteSheet(sheet), mSubimageRect(subimageRect)
    {
        
    }
    
public:
    void draw (int x, int y, int w, int h) const {
        mSpriteSheet.draw(mSubimageRect,{x,y,w,h});
    }
    
    void draw (int x, int y, int w, int h, double angle) const {
        mSpriteSheet.draw(mSubimageRect, {x,y,w,h}, angle, NULL);
    }
    
private:
    SpriteSheet mSpriteSheet;
    SDL_Rect mSubimageRect;
};

#endif
