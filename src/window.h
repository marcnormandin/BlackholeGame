#ifndef BlackHole_window_h
#define BlackHole_window_h

#include <SDL2/SDL.h>

class Window {
public:
    Window (const std::string& windowTitle, int posX, int posY, int width, int height)
    {
        //Use OpenGL 2.1
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
        
        //Create window
        mWindow = SDL_CreateWindow( "Blaster Master", posX, posY, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
        if( mWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            throw std::runtime_error("Unable to create OPENGL window.");
        }        
    }
    
    ~Window() {
        SDL_DestroyWindow (mWindow);
    }
    
    SDL_Window* getWindow() { return mWindow; }
    
private:
    SDL_Window*     mWindow;
};

#endif
