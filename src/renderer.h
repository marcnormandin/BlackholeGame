#ifndef BlackHole_renderer_h
#define BlackHole_renderer_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "window.h"
#include "texture.h"
#include "spritesheet.h"
#include "spriteanimation.h"
#include <memory>

class Renderer {
public:
    Renderer (Window* window)
    : mRenderer(NULL)
    {
        mContext = SDL_GL_CreateContext( window->getWindow() );
        if (mContext == NULL) {
            throw std::runtime_error("Unabel to create opengl context.");
        }
        
        SDL_GL_SetSwapInterval( 1 );
        
        if (!initGL()) {
            throw std::runtime_error("Error initializing opengl");
        }
        mRenderer = SDL_CreateRenderer (window->getWindow(), -1, SDL_RENDERER_SOFTWARE);
    }
    
    ~Renderer() {
        SDL_DestroyRenderer (mRenderer);
    }
    
    SDL_Renderer* getRenderer() {
        return mRenderer;
    }
    
    void beginFrame() {
        SDL_RenderClear (mRenderer);
    }
    
    void endFrame() {
        SDL_RenderPresent (mRenderer);
    }
    
    std::shared_ptr<Texture> createTexture(const std::string& filename) {
        return std::make_shared<Texture>(this->getRenderer(), filename);
    }
    
    std::shared_ptr<SpriteSheet> createSpriteSheet(const std::string& filename) {
        return std::make_shared<SpriteSheet>(this->getRenderer(), filename);
    }
    
    std::shared_ptr<SpriteAnimation> createSpriteAnimation(const std::string& filename,
                                                           const int numWidth, const int numHeight) {
        std::shared_ptr<SpriteSheet> spriteSheet = this->createSpriteSheet( filename );
        return std::make_shared<SpriteAnimation>(spriteSheet, numWidth, numHeight);
    }
    
    void draw (Texture& texture) const {
        SDL_RenderCopy (mRenderer, texture.getSDLTexture(), NULL, NULL);
    }
    
protected:
    int initGL() {
        bool success = true;
        GLenum error = GL_NO_ERROR;
        
        //Initialize Projection Matrix
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        
        //Check for error
        error = glGetError();
        if( error != GL_NO_ERROR )
        {
            success = false;
        }
        
        //Initialize Modelview Matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();
        
        //Check for error
        error = glGetError();
        if( error != GL_NO_ERROR )
        {
            success = false;
        }
        
        //Initialize clear color
        glClearColor( 0.f, 0.f, 0.f, 1.f );
        
        //Check for error
        error = glGetError();
        if( error != GL_NO_ERROR )
        {
            success = false;
        }
        
        return success;
    }
    
private:
    SDL_GLContext mContext;
    SDL_Renderer*   mRenderer;
};

#endif
