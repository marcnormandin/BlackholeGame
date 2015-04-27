#ifndef BlackHole_texture_h
#define BlackHole_texture_h

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <string>
#include <sstream>

typedef std::shared_ptr<SDL_Texture> SharedSDLTexture;

// Custom "deleter"
// http://stackoverflow.com/questions/18682868/assigning-stdshared-ptr
inline SharedSDLTexture make_shared(SDL_Texture* texture) {
    return SharedSDLTexture(texture, SDL_DestroyTexture);
}

class Texture {
public:
    // Create a teture from an image file
    Texture (SDL_Renderer* renderer, const std::string& filename)
    {
        // Create the surface from the image
        SDL_Surface* surface = IMG_Load( filename.c_str() );

        if (!surface)
        {
            std::ostringstream oss;
            oss << "(" << __FILE__ << ":" << __LINE__ << "): ";
            oss << "Unable to create surface from image file (" << filename << ")";
            throw std::runtime_error(oss.str());
        }
    
        SDL_SetColorKey (surface, SDL_TRUE, SDL_MapRGBA(surface->format, 255, 255, 255,255));

        // Convert surface to texture
        mTexture = make_shared(SDL_CreateTextureFromSurface(renderer, surface));
        if (!mTexture)
        {
            SDL_FreeSurface(surface);
            
            std::ostringstream oss;
            oss << "(" << __FILE__ << ":" << __LINE__ << "): ";
            oss << "Unable to create texture from surface." << filename;
            throw std::runtime_error(oss.str());
        }
    
        SDL_FreeSurface(surface);
    }
    
    ~Texture() {
    }
    
    SDL_Texture* getSDLTexture() const {
        return mTexture.get();
    }
    
private:
    SharedSDLTexture mTexture;
};

#endif
