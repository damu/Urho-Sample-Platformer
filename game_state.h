#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include "misc.h"

class game_state;

/// For stuff you want to share across game states.
class globals
{
public:
    Urho3D::Camera* camera=0;
    Urho3D::Scene* scene=0;
    Urho3D::ResourceCache* cache=0;
    Urho3D::Context* context=0;
    Urho3D::UIElement* ui_root=0;
    Urho3D::Engine* engine=0;
    Urho3D::PhysicsWorld* physical_world=0;

    /// The current game states, so that game states can switch to another game state.
    /// Watch out that changing a game state will delete the current one.
    /// Also game states can be stacked on top of each other so they run parallel. This is used for the pause mode.
    std::vector<std::unique_ptr<game_state>> game_states;

    /// Meyer Singleton
    static globals* instance()
    {
        static globals g;
        return &g;
    }
};

/// For a game state system that can run multiple game states at the same time.
class game_state : public Urho3D::Object
{
public:
    std::vector<Urho3D::Node*> nodes;   ///< Put all your nodes in this vector so that they are removed on game state deletion.
    std::vector<Urho3D::UIElement*> gui_elements;   ///< Put all your GUI elements here so that they are removed on game state deletion.
    // there could be other vectors here with stuff that will be automatically removed in the destructor

    game_state() : Urho3D::Object(globals::instance()->context) {}
    /// Removes all GUI elements and all nodes in the 'nodes' vector.
    virtual ~game_state()
    {
        for(auto e:gui_elements)
            e->Remove();
        gui_elements.clear();
        for(auto n:nodes)
            n->Remove();
        nodes.clear();
    }

    URHO3D_OBJECT(game_state,Urho3D::Object)
};

#endif // GAME_STATE_H
