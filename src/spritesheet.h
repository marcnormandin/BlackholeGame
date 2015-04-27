#ifndef BlackHole_spritesheet_h
#define BlackHole_spritesheet_h

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <string>

//#include "sprite.h"
#include "texture.h"

class Sprite;

class SpriteSheet {
public:
    friend class Renderer;
    
    // Called by Renderer
    SpriteSheet (SDL_Renderer* renderer, const std::string& filename)
    : mTexture(renderer, filename), mRenderer(renderer)
    {
        SDL_QueryTexture(mTexture.getSDLTexture(), NULL, NULL, &mWidth, &mHeight);
    }
    
public:
    
    virtual ~SpriteSheet()
    {
    }
    
    Sprite createSprite(int x, int y, int w, int h);
    
    void draw (const SDL_Rect& src, const SDL_Rect& dest) const
    {
        SDL_RenderCopy (mRenderer, mTexture.getSDLTexture(), &src, &dest);
    }
    
    void draw (const SDL_Rect& src, const SDL_Rect& dest, const double angle, const SDL_Point* center) const
    {
        SDL_RenderCopyEx (mRenderer, mTexture.getSDLTexture(), &src, &dest, angle, center, SDL_FLIP_NONE );
    }
    
    int width() const {
        return mWidth;
    }
    
    int height() const {
        return mHeight;
    }
    
protected:

    
private:
    Texture mTexture;
    int mWidth, mHeight;
    SDL_Renderer* mRenderer;
};

#endif
