#ifndef BlackHole_game_h
#define BlackHole_game_h

#include <SDL2/SDL.h>
// http://stackoverflow.com/questions/22368202/xcode-5-crashes-when-running-an-app-with-sdl-2

#include "entitysystem.h"
#include "renderer.h"
#include "window.h"
#include "sprite.h"
#include "spriteanimation.h"
#include <chrono>
#include <memory>
#include <random>
#include <utility>
#include "soundsystem.h"
#include <cmath>

class Vector2f {
public:
    Vector2f(float X = 0.0, float Y = 0.0) : x(X), y(Y) {}
    float x, y;
};

class Game {
    enum EntityGroups : std::size_t {
        EG_BLACKHOLE,
        EG_PHOTONTORPEDO,
        EG_SPACESHIP,
        EG_HUMANSPACESHIP,
        EG_EXPLOSION,
        EG_ASTEROID,
        EG_DESTROYABLE
    };
    
    // Entities can have a position in the game world.
    struct CPosition : EntitySystem::Component
    {
        Vector2f position;
    
        CPosition() = default;
        CPosition(const Vector2f& mPosition) : position(mPosition) { }
        
        float x() const noexcept { return position.x; }
        float y() const noexcept { return position.y; }
    };
    
    struct CDirection : EntitySystem::Component
    {
        float mAngle;
        
        CDirection() = default;
        CDirection(const float angle) : mAngle(angle) {}
        
        float angle() const noexcept { return mAngle; }
        void setAngle( float angle ) noexcept {
            mAngle = angle;
        }
    };
    
    // Entities can have a physical body and a velocity.
    struct CLinearPhysics : EntitySystem::Component
    {
        using Bound = std::pair<float,float>;
        
        CPosition* mPosition{nullptr};
        CDirection* mDirection{nullptr};
        Vector2f mVelocity, mHalfSize;
        Bound mBoundX, mBoundY;
        float mSpeed;
        
        // We will use a callback to handle the "out of bounds" event.
        std::function<void(const Vector2f&)> onOutOfBounds;
        
        CLinearPhysics(const Vector2f& velocity, const Vector2f& halfSize, const Bound& boundX, const Bound& boundY)
        : mVelocity(velocity), mHalfSize(halfSize), mBoundX(boundX), mBoundY(boundY) {
            mSpeed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);
        }
        
        void init() override
        {
            // A requirement for `CPhysics` is obviously `CPosition`.
            mPosition = &entity->getComponent<CPosition>();
            mDirection = &entity->getComponent<CDirection>();
        }
        
        void update(float ft) override
        {
            // scale the velocity vector since it may have changed (from the blackhole)
            float currentSpeed = std::sqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);
            mVelocity.x /= currentSpeed;
            mVelocity.y /= currentSpeed;
            
            mVelocity.x *= mSpeed;
            mVelocity.y *= mSpeed;

            mPosition->position.x += mVelocity.x * ft;
            mPosition->position.y += mVelocity.y * ft;

            float angleRad = std::atan2(mVelocity.y, mVelocity.x);
            float angleDeg = angleRad * 180.0 / M_PI;
            float newAngleDeg = angleDeg + 90;
            mDirection->setAngle( newAngleDeg );
            
            if(onOutOfBounds == nullptr) return;
            
            if(left() < mBoundX.first) onOutOfBounds(Vector2f{1.f, 0.f});
            else if(right() > mBoundX.second) onOutOfBounds(Vector2f{-1.f, 0.f});
            
            if(top() < mBoundY.first) onOutOfBounds(Vector2f{0.f, 1.f});
            else if(bottom() > mBoundY.second) onOutOfBounds(Vector2f{0.f, -1.f});
        }
        
        float x() 		const noexcept { return mPosition->x(); }
        float y() 		const noexcept { return mPosition->y(); }
        float left() 	const noexcept { return x() - mHalfSize.x; }
        float right() 	const noexcept { return x() + mHalfSize.x; }
        float top() 	const noexcept { return y() - mHalfSize.y; }
        float bottom() 	const noexcept { return y() + mHalfSize.y; }
    };
    
    // An entity can be drawn with a sprite
    struct CSprite : EntitySystem::Component
    {
        CPosition* mPosition;
        CDirection* mDirection;
        
        Sprite mSprite;
        SDL_Rect mRect;
        float mWidth, mHeight;
        float mAngle;
        
        CSprite(Sprite sprite, float width, float height)
        : mSprite(sprite), mWidth(width), mHeight(height) {
        }
        
        virtual ~CSprite() {
        }
        
        void init() override
        {
            mPosition = &entity->getComponent<CPosition>();
            mDirection = &entity->getComponent<CDirection>();
            
            update(0.0);
        }
        
        void update(float mFT) override
        {
            mRect.x = mPosition->x() - mWidth/2.0;
            mRect.y = mPosition->y() - mHeight/2.0;
            mRect.w = mWidth;
            mRect.h = mHeight;
            
            mAngle = mDirection->angle();
        }
        
        void draw() override
        {
            mSprite.draw(mRect.x, mRect.y, mRect.w, mRect.h, mAngle);
        }
    };
    
    struct CSpriteAnimation : EntitySystem::Component
    {
        CPosition* mPosition;
        
        std::shared_ptr<SpriteAnimation> mSpriteAnimation;
        SDL_Rect mRect;
        float mWidth, mHeight;
        float mDuration;
        float mTimeAlive{0.0};
        int mCurrentFrame{0};
        bool mKillOnLastFrame{true};
        
        CSpriteAnimation(std::shared_ptr<SpriteAnimation> spriteAnimation, float width, float height, float duration, bool killOnLastFrame = true)
        : mSpriteAnimation(spriteAnimation), mWidth(width), mHeight(height), mDuration(duration),
        mKillOnLastFrame(killOnLastFrame) {
        }
        
        virtual ~CSpriteAnimation() {
        }
        
        void init() override
        {
            mPosition = &entity->getComponent<CPosition>();
            
            update(0.0);
        }
        
        void update(float ft) override
        {
            mRect.x = mPosition->x() - mWidth/2.0;
            mRect.y = mPosition->y() - mHeight/2.0;
            mRect.w = mWidth;
            mRect.h = mHeight;
            
            
            
            float secs_per_frame = mDuration / (1.0f*mSpriteAnimation->numFrames());
            if (mTimeAlive > (secs_per_frame * (mCurrentFrame))) {
                nextFrame();
            }
            
            if (mTimeAlive > mDuration) {
                if (mKillOnLastFrame) {
                    entity->destroy();
                }
                
                // Reset the animation
                mTimeAlive = 0.0;
                mCurrentFrame = 0;
            }
            
            mTimeAlive += ft;
        }
        
        void draw() override
        {
            mSpriteAnimation->draw(mRect.x, mRect.y, mRect.w, mRect.h, mCurrentFrame);
        }
        
    protected:
        //int currentFrame() const { return mCurrentFrame; }
        void nextFrame() {
            mCurrentFrame += 1;
            
            // return back to zero
            if (mCurrentFrame >= mSpriteAnimation->numFrames() - 1) {
                mCurrentFrame = 0;
            }
        }
    };
    
    
    // To-do: Render the square to a texture, then display the rotated texture
    struct CRectangle : EntitySystem::Component
    {
        CPosition* mPosition;
        CDirection* mDirection;
        Game* mGame;
        SDL_Rect mRect;
        float mWidth{2.0};
        float mHeight{2.0};
        
        CRectangle(Game* game, float width = 2.0f, float height = 2.0f)
        : mGame(game), mWidth(width), mHeight(height) {}
        
        void init() override
        {
            mPosition = &entity->getComponent<CPosition>();
            mDirection = &entity->getComponent<CDirection>();
        }
        
        void update(float ft) override
        {
            mRect.x = mPosition->x() - mWidth/2.0;
            mRect.y = mPosition->y() - mHeight/2.0;
            
            // Hardcoded dimensions
            mRect.w = mWidth;
            mRect.h = mHeight;
        }
        
        void draw() override
        {
            // Hardcoded color
            SDL_SetRenderDrawColor( mGame->mRenderer->getRenderer(), 0, 255, 0, 255 );
            
            // Render rect
            SDL_RenderFillRect( mGame->mRenderer->getRenderer(), &mRect );
        }
    };
    
    
    // Entities can have a physical body and a velocity.
    struct CCollisionBox : EntitySystem::Component
    {
        CPosition* mPosition{nullptr};
        Vector2f mHalfSize;
        
        CCollisionBox(const Vector2f& halfSize)
        : mHalfSize(halfSize) { }
        
        void init() override
        {
            // A requirement for `CPhysics` is obviously `CPosition`.
            mPosition = &entity->getComponent<CPosition>();
        }
        
        void update(float ft) override
        {
        }
        
        float x() 		const noexcept { return mPosition->x(); }
        float y() 		const noexcept { return mPosition->y(); }
        float left() 	const noexcept { return x() - mHalfSize.x; }
        float right() 	const noexcept { return x() + mHalfSize.x; }
        float top() 	const noexcept { return y() - mHalfSize.y; }
        float bottom() 	const noexcept { return y() + mHalfSize.y; }
    };
    
    
    
    // AI will control 'input'
    struct CInputAI : EntitySystem::Component
    {
        CPosition* mPosition;
        CDirection* mDirection;
        
        float mAngleSpeedPerSec;
        
        CInputAI(float angleSpeedDegPerSec = 2.0)
        : mAngleSpeedPerSec(angleSpeedDegPerSec)
        {
        }
        
        void init() override
        {
            mPosition = &entity->getComponent<CPosition>();
            mDirection = &entity->getComponent<CDirection>();
        }
        
        void update(float mFT) override
        {
            float angleChange = mAngleSpeedPerSec * mFT;
            float oldAngle = mDirection->angle();
            float newAngle = oldAngle + angleChange;
            
            if(newAngle < 0.0) {
                while(newAngle < 0.0) newAngle += 360.0;
            } else if (newAngle > 360.0) {
                while (newAngle >= 360.0) newAngle -= 360.0;
            }
            
            mDirection->setAngle(newAngle);
        }
        
        void draw() override
        {
        }
    };
    
    
    struct CInputHuman : EntitySystem::Component
    {
        Game* mGame; // needed to create a photon
        CDirection* mDirection;
        
        float mAngleSpeedPerSec;
        
        enum RotationDirection {
            RD_LEFT,
            RD_NONE,
            RD_RIGHT
        };
        
        RotationDirection mRotation;
        
        CInputHuman(Game* game, float angleSpeedDegPerSec = 120.0)
        : mGame(game), mAngleSpeedPerSec(angleSpeedDegPerSec), mRotation(RD_NONE)
        {
        }
        
        void init() override
        {
            mDirection = &entity->getComponent<CDirection>();
        }
        
        void update(float mFT) override {
            float angleChange = mAngleSpeedPerSec * mFT;
            mRotation = RD_NONE;
            
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    mGame->mIsRunning = false;
                    return;
                }
                else if( e.type == SDL_KEYDOWN )
                {
                    //Select surfaces based on key press
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_UP:
                            auto& position(entity->getComponent<CPosition>());
                            auto& direction(entity->getComponent<CDirection>());
                            mGame->createPhotonTorpedo(position.x(), position.y(), direction.angle());
                            mGame->mSoundSystem->playFire();
                            break;
                    }
                }
            }
            
            const Uint8* currentKeyStates = SDL_GetKeyboardState( NULL );
 
            if( currentKeyStates[ SDL_SCANCODE_LEFT ] )
            {
                mRotation = RD_LEFT;
            }
            else if( currentKeyStates[ SDL_SCANCODE_RIGHT ] )
            {
                mRotation = RD_RIGHT;
            }
            
            if (mRotation == RD_LEFT) angleChange *= -1.0;
            else if (mRotation == RD_NONE) angleChange = 0.0;
            
            float oldAngle = mDirection->angle();
            float newAngle = oldAngle + angleChange;
            
            if(newAngle < 0.0) {
                while(newAngle < 0.0) newAngle += 360.0;
            } else if (newAngle > 360.0) {
                while (newAngle >= 360.0) newAngle -= 360.0;
            }
            
            mDirection->setAngle(newAngle);
        }
    };
    
    // This only works with intersecting rectangles
    template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB) noexcept
    {
        return mA.right() >= mB.left() && mA.left() <= mB.right()
        && mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
    }
    
public:
    Game() {
        SDL_Init (SDL_INIT_EVERYTHING);
        
        // load support for the JPG and PNG image formats
        int flags=IMG_INIT_JPG|IMG_INIT_PNG;
        int initted=IMG_Init(flags);
        if((initted&flags) != flags) {
            printf("IMG_Init: Failed to init required jpg and png support!\n");
            printf("IMG_Init: %s\n", IMG_GetError());
            // handle error
        }
        
        mWindow = new Window ("operation touchdown", 0, 0, mWindowWidth, mWindowHeight);
        mRenderer = new Renderer (mWindow);
        mSpaceshipSS = mRenderer->createSpriteSheet("spaceships.png");
        mPhotonSS = mRenderer->createSpriteSheet("rocketTrail.png");
        mSpaceshipBlue = mRenderer->createSpriteSheet("blueships1.png");
        mBackground = mRenderer->createTexture("background.bmp");
        mExplosionAnimation = mRenderer->createSpriteAnimation("explode_3.png", 4, 4);
        mAsteroidAnimation = mRenderer->createSpriteAnimation("asteroid1.png", 5, 4);

        createHumanSpaceship();
        
        // For fun, create a bunch of random AI controlled spaceships
        {
            // Seed with a real random value, if available
            std::random_device rd;
            
            // Choose a random mean between 1 and 6
            std::default_random_engine e1(rd());
            std::uniform_int_distribution<int> randomX(80, mWindowWidth-80);
            std::uniform_int_distribution<int> randomY(80, mWindowHeight-80);
            
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> randomRotationSpeed(-359.0, 359.0);

            // Create random spaceships
            for (int i = 0; i < 25; i++) {
                int posX = randomX(e1);
                int posY = randomY(e1);
                double rotationSpeed = randomRotationSpeed(gen);
                createAISpaceship( posX, posY, rotationSpeed );
            }
            
            // Create random asteroids
            for (int i = 0; i < 25; i++) {
                int posX = randomX(e1);
                int posY = randomY(e1);
                //double rotationP = randomRotationSpeed(gen);
                createAsteroid( posX, posY );
            }
        }
        
        mIsRunning = false;
        
        mSoundSystem = new SoundSystem();
    }
    
    ~Game() {
        delete mRenderer;
        delete mWindow;
        delete mSoundSystem;
        
        SDL_Quit();
        IMG_Quit();
    }
    
    void run ()
    {
        gameLoop();
    }
    
    Window* getWindow();
    Renderer* getRenderer();
    
protected:
    // See the "Game Update" pattern
    // http://gameprogrammingpatterns.com/game-loop.html
    void gameLoop ()
    {
        using FrameTime = float;
        
        mIsRunning = true;
        
        auto previousTime(std::chrono::high_resolution_clock::now());
        float lag = 0.0;
        
        // Note: The "Game Update" pattern uses milliseconds per frame
        const double FRAMES_PER_SECOND = 60.0;
        
        const double SECONDS_PER_UPDATE = (1.0 / FRAMES_PER_SECOND);
        
        while (mIsRunning)
        {
            auto currentTime(std::chrono::high_resolution_clock::now());
            auto elapsedTimeMS = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - previousTime);
            
            previousTime = currentTime;
            
            // convert to seconds
            lag += elapsedTimeMS.count() / 1000.0;
            
            handleInput();
            
            while (lag >= SECONDS_PER_UPDATE)
            {
                update( SECONDS_PER_UPDATE);
                lag -= SECONDS_PER_UPDATE;
                // Check for collisions
                // We get our entities by group...
                auto& spaceships(mManager.getEntitiesByGroup(EG_DESTROYABLE));
                auto& photons(mManager.getEntitiesByGroup(EG_PHOTONTORPEDO));
                
                // Apply gravity to the photons
                // THIS IS NOT THE BEST PLACE! HACK HACK HACK
                for ( auto& photon : photons ) {
                    float bh_x = mWindowWidth / 2.0;
                    float bh_y = mWindowHeight / 2.0;
                    
                    // This is ridiculous!
                    auto& pp(photon->getComponent<CPosition>());
                    auto& plp(photon->getComponent<CLinearPhysics>());
                    float dx = bh_x - pp.x();
                    float dy = bh_y - pp.y();
                    float d = std::sqrt( (bh_x - pp.x()) * (bh_x - pp.x()) + (bh_y - pp.y()) * (bh_y - pp.y()) );
                    float s = 5000000.0f / (d*d);
                    float sdx = dx / d;
                    float sdy = dy / d;
                    float ax = sdx * s;
                    float ay = sdy * s;
                    float vx = ax * SECONDS_PER_UPDATE;
                    float vy = ay * SECONDS_PER_UPDATE;
                    plp.mVelocity.x += vx;
                    plp.mVelocity.y += vy;
                }
                
                // Collision handling
                for ( auto& photon : photons ) {
                    auto& pphoton(photon->getComponent<CCollisionBox>());
                 
                    for ( auto& spaceship : spaceships ) {
                        auto& pspaceship( spaceship->getComponent<CCollisionBox>());
                        if (isIntersecting(pphoton, pspaceship)) {
                            spaceship->destroy();
                            
                            auto& pos(spaceship->getComponent<CPosition>());
                            createExplosion(pos.position.x, pos.position.y);
                            this->mSoundSystem->playExplosion();
                            photon->destroy();
                            
                            break;
                        }
                    }
                }
            }
            
            //mManager.refresh();

            
            draw();
        }
        
    }
    
    void handleInput () {
        
        // The Input components will handle the rest
    }
    
    void draw () {
        mRenderer->beginFrame();
        
        
        mRenderer->draw(*mBackground);
        
        mManager.draw();
        
        mRenderer->endFrame();
    }
    
    void update(float seconds) {
        mManager.refresh();
        mManager.update( seconds );
    }
    
protected:
    EntitySystem::Entity& createHumanSpaceship()
    {
        auto& entity(mManager.addEntity());
        entity.addComponent<CPosition>(Vector2f{100.0, mWindowHeight/2.0f});
        entity.addComponent<CDirection>();
        CLinearPhysics::Bound boundX{20.0f,1.0f*mWindowWidth-20};
        CLinearPhysics::Bound boundY{20.0f,1.0f*mWindowHeight-20};
    
        entity.addComponent<CCollisionBox>(Vector2f(40,50));
        entity.addComponent<CSprite>(mSpaceshipBlue->createSprite(22, 46, 700, 900), 20, 25);
        
        // Human controlled
        // This class is currently buggy! TO FIX!
        entity.addComponent<CInputHuman>(this, 240.0);
        
        entity.addGroup(EntityGroups::EG_HUMANSPACESHIP);
        
        return entity;
    }
    
    EntitySystem::Entity& createAISpaceship(int posX, int posY, float rotationSpeed)
    {
        auto& entity(mManager.addEntity());
        entity.addComponent<CPosition>(Vector2f{1.0f*posX, 1.0f*posY});
        entity.addComponent<CDirection>();
        Vector2f halfSize{10,10};
        entity.addComponent<CCollisionBox>(Vector2f(halfSize.x,halfSize.y));
        entity.addComponent<CSprite>(mSpaceshipSS->createSprite(840, 0, 610, 530), 2*halfSize.x, 2*halfSize.y);

        entity.addComponent<CInputAI>(rotationSpeed);
        
        entity.addGroup(EntityGroups::EG_SPACESHIP);
        entity.addGroup(EntityGroups::EG_DESTROYABLE);
        
        return entity;
    }
    
    EntitySystem::Entity& createAsteroid(int posX, int posY)
    {
        auto& entity(mManager.addEntity());
        entity.addComponent<CPosition>(Vector2f{1.0f*posX, 1.0f*posY});
        entity.addComponent<CDirection>();
        Vector2f halfSize{20,20};
        entity.addComponent<CCollisionBox>(Vector2f(halfSize.x,halfSize.y));
        entity.addComponent<CSpriteAnimation>(mAsteroidAnimation, 40, 40, 2, false);

        entity.addGroup(EntityGroups::EG_ASTEROID);
        entity.addGroup(EntityGroups::EG_DESTROYABLE);
        
        return entity;
    }
    
    EntitySystem::Entity& createPhotonTorpedo(int posX, int posY, float angle)
    {
        auto& entity(mManager.addEntity());
        entity.addComponent<CPosition>(Vector2f{1.0f*posX, 1.0f*posY});
        entity.addComponent<CDirection>(angle);
        
        // This shouldn't be needed. Should be able to specify it using just {}
        float speed = 250.0f;
        
        float angleRad = angle * (M_PI / 180.0);
        Vector2f velocity{ std::sin(angleRad) * speed, -std::cos(angleRad) * speed };
        Vector2f halfSize{2.0,6.0};
        CLinearPhysics::Bound boundX{20.0f,1.0f*mWindowWidth-20};
        CLinearPhysics::Bound boundY{20.0f,1.0f*mWindowHeight-20};
        
        entity.addComponent<CLinearPhysics>(velocity,halfSize,boundX,boundY);
        entity.addComponent<CCollisionBox>(Vector2f(halfSize.x,halfSize.y));
        entity.addComponent<CSprite>(mPhotonSS->createSprite(0, 0, 28, 86), halfSize.x*2.0, halfSize.y*2.0);

        auto& cPhysics(entity.getComponent<CLinearPhysics>());
        
        cPhysics.onOutOfBounds = [&cPhysics](const Vector2f& mSide)
        {
            cPhysics.entity->destroy();
        };
        
        entity.addGroup(EntityGroups::EG_PHOTONTORPEDO);
        
        return entity;
    }
    
    EntitySystem::Entity& createExplosion(int posX, int posY)
    {
        auto& entity(mManager.addEntity());
        entity.addComponent<CPosition>(Vector2f{1.0f*posX, 1.0f*posY});
        
        entity.addComponent<CSpriteAnimation>(mExplosionAnimation, 60, 60, 2);
        
        entity.addGroup(EntityGroups::EG_EXPLOSION);
        
        return entity;
    }
    
    
private:
    int mWindowWidth{1024};
    int mWindowHeight{768};
    Window* mWindow;
    Renderer* mRenderer;
    bool mIsRunning;
    
    std::shared_ptr<SpriteSheet> mSpaceshipSS;
    std::shared_ptr<SpriteSheet> mPhotonSS;
    std::shared_ptr<SpriteSheet> mSpaceshipBlue;
    std::shared_ptr<Texture> mBackground;
    std::shared_ptr<SpriteAnimation> mExplosionAnimation;
    std::shared_ptr<SpriteAnimation> mAsteroidAnimation;

    
    EntitySystem::Manager mManager;
    
    SoundSystem* mSoundSystem;
};

#endif
