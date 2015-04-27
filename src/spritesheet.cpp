#include <stdio.h>

#include "spritesheet.h"
#include "sprite.h"

#include <memory>

Sprite SpriteSheet::createSprite(int x, int y, int w, int h)
{
    return Sprite(*this, {x,y,w,h} );
}
