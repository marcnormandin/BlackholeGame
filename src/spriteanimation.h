#ifndef BlackHole_spriteanimation_h
#define BlackHole_spriteanimation_h

#include "spritesheet.h"
#include <vector>

class SpriteAnimation {
public:
    SpriteAnimation(std::shared_ptr<SpriteSheet> spriteSheet, const int numWidth, const int numHeight)
    : mSpriteSheet(spriteSheet), mNumSpritesWidth(numWidth), mNumSpritesHeight(numHeight)
    {
        int totalWidth = mSpriteSheet->width();
        int totalHeight = mSpriteSheet->height();
        
        mFramePixelWidth = totalWidth / numWidth; // !Fixeme
        mFramePixelHeight = totalHeight / numHeight;
        
        // frames are sequential from left to right, then top to bottom
        for (int y = 0; y < numHeight; ++y) {
            for (int x = 0; x < numWidth; x++) {
                SDL_Rect rect;
                rect.x = x * mFramePixelWidth;
                rect.y = y * mFramePixelHeight;
                rect.w = mFramePixelWidth;
                rect.h = mFramePixelHeight;
                
                mRects.push_back(rect);
            }
        }
    }
    
    int numFrames() const { return mNumSpritesWidth * mNumSpritesHeight; }
    
    void draw(int x, int y, int w, int h, int frame) {
        mSpriteSheet->draw( mRects[frame], {x,y,w,h} );
    }
    
    int framePixelWidth() const {
        return mFramePixelWidth;
    }
    
    int framePixelHeight() const {
        return mFramePixelHeight;
    }
    
protected:
    
private:
    std::shared_ptr<SpriteSheet> mSpriteSheet;
    int mSpriteSheetWidth, mSpriteSheetHeight;
    int mNumSpritesWidth, mNumSpritesHeight;
    int mCurrentFrame{0};
    int mFramePixelWidth, mFramePixelHeight;
    std::vector<SDL_Rect> mRects;
};

#endif
