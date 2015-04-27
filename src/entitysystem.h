// Copyright (c) 2013-2015 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

// Modified to work with our game.

#ifndef ENTITYSYSTEM_H
#define ENTITYSYSTEM_H

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>
#include <cassert>
#include <type_traits>
#include <cmath>
#include <chrono>
#include <functional>

namespace EntitySystem
{
    struct Component;
    class Entity;
    class Manager;
    
    using ComponentID = std::size_t;
    using Group = std::size_t;
    
    namespace Internal
    {
        inline ComponentID getUniqueComponentID() noexcept
        {
            static ComponentID lastID{0u};
            return lastID++;
        }
    }
    
    template<typename T> inline ComponentID getComponentTypeID() noexcept
    {
        static_assert(std::is_base_of<Component, T>::value,
                      "T must inherit from Component");
        
        static ComponentID typeID{Internal::getUniqueComponentID()};
        return typeID;
    }
    
    constexpr std::size_t maxComponents{32};
    using ComponentBitset = std::bitset<maxComponents>;
    using ComponentArray = std::array<Component*, maxComponents>;
    
    constexpr std::size_t maxGroups{32};
    using GroupBitset = std::bitset<maxGroups>;
    
    struct Component
    {
        Entity* entity;
        
        virtual void init() { }
        virtual void update(float mFT) { }
        virtual void draw() { }
        
        virtual ~Component() { }
    };
    
    class Entity
    {
    private:
        Manager& manager;
        
        bool alive{true};
        std::vector<std::unique_ptr<Component>> components;
        ComponentArray componentArray;
        ComponentBitset componentBitset;
        
        GroupBitset groupBitset;
        
    public:
        Entity(Manager& mManager) : manager(mManager) { }
        
        void update(float mFT) 	{ for(auto& c : components) c->update(mFT); }
        void draw() 			{ for(auto& c : components) c->draw(); }
        
        bool isAlive() const 	{ return alive; }
        void destroy() 			{ alive = false; }
        
        template<typename T> bool hasComponent() const
        {
            return componentBitset[getComponentTypeID<T>()];
        }
        
        bool hasGroup(Group mGroup) const noexcept
        {
            return groupBitset[mGroup];
        }
        
        void addGroup(Group mGroup) noexcept;
        void delGroup(Group mGroup) noexcept
        {
            groupBitset[mGroup] = false;
        }
        
        template<typename T, typename... TArgs>
        T& addComponent(TArgs&&... mArgs)
        {
            assert(!hasComponent<T>());
            
            T* c(new T(std::forward<TArgs>(mArgs)...));
            c->entity = this;
            std::unique_ptr<Component> uPtr{c};
            components.emplace_back(std::move(uPtr));
            
            componentArray[getComponentTypeID<T>()] = c;
            componentBitset[getComponentTypeID<T>()] = true;
            
            c->init();
            return *c;
        }
        
        template<typename T> T& getComponent() const
        {
            assert(hasComponent<T>());
            auto ptr(componentArray[getComponentTypeID<T>()]);
            return *reinterpret_cast<T*>(ptr);
        }
    };
    
    struct Manager
    {
    private:
        std::vector<std::unique_ptr<Entity>> entities;
        std::array<std::vector<Entity*>, maxGroups> groupedEntities;
        
    public:
        void update(float ft) 	{
            // The loop was changed to the old style so that
            // we could use indices, because some entities
            // create other entities, which may screw up the previous loop
            std::size_t N = entities.size();
            for (int n = 0; n < N; n++) {
                entities[n]->update( ft );
            }
        }
        void draw() 			{ for(auto& e : entities) e->draw(); }
        
        void addToGroup(Entity* mEntity, Group mGroup)
        {
            groupedEntities[mGroup].emplace_back(mEntity);
        }
        
        std::vector<Entity*>& getEntitiesByGroup(Group mGroup)
        {
            return groupedEntities[mGroup];
        }
        
        void refresh()
        {
            for(auto i(0u); i < maxGroups; ++i)
            {
                auto& v(groupedEntities[i]);
                
                v.erase(
                        std::remove_if(std::begin(v), std::end(v),
                                       [i](Entity* mEntity)
                                       {
                                           return !mEntity->isAlive() || !mEntity->hasGroup(i);
                                       }),
                        std::end(v));
            }
            
            entities.erase(
                           std::remove_if(std::begin(entities), std::end(entities),
                                          [](const std::unique_ptr<Entity>& mEntity)
                                          {
                                              return !mEntity->isAlive();
                                          }),
                           std::end(entities));
        }
        
        Entity& addEntity()
        {
            Entity* e(new Entity(*this));
            std::unique_ptr<Entity> uPtr{e};
            entities.emplace_back(std::move(uPtr));
            return *e;
        }
    };
    
    void Entity::addGroup(Group mGroup) noexcept
    {
        groupBitset[mGroup] = true;
        manager.addToGroup(this, mGroup);
    }
} // namespace EntitySystem

#endif // #ifndef ENTITYSYSTEM_H
    
    
