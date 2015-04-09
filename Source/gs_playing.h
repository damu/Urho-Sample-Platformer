#ifndef GS_PLAYING_H
#define GS_PLAYING_H

#include "game_state.h"

#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/IO/Log.h>

class gs_playing : public game_state
{
public:
    int framecount_=0;
    float time_=0;
    Urho3D::Text* text_;
    Urho3D::Node* node_player;
    Urho3D::Node* node_player_model;
    Urho3D::RigidBody* body_player;
    float cam_distance=8;
    timer timer_playing;
    float goal_time=0;
    std::vector<Urho3D::Vector3> flag_positions;
    std::vector<Urho3D::Node*> flag_nodes;

    gs_playing();
    void update(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleKeyDown(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);

    virtual const Urho3D::String& GetTypeName() const {static Urho3D::String name("gs_playing");return name;}   // this could be correct
};

#endif // GS_PLAYING_H
