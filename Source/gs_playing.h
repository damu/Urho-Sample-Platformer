#ifndef GS_PLAYING_H
#define GS_PLAYING_H

#include "game_state.h"

#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
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
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>

class player;

/// A BoundingBox shaped trigger (trigger_area) that spawns rocks inside the spawn_area BoundingBox.
struct level_rock_spawn
{
    Urho3D::BoundingBox trigger_area;
    Urho3D::BoundingBox spawn_area;
    int rock_count=100;
    bool rocks_spawned=false;           ///< if the trigger has already been activated
};

struct level_static_model
{
    Urho3D::String name;
    Urho3D::Vector3 pos;

    level_static_model(){}
    level_static_model(Urho3D::String name,Urho3D::Vector3 pos) : name(name),pos(pos) {}
};

class gs_playing;

/// Level class used to store all map related data that are read from the XML map files.
class level
{
public:
    std::vector<Urho3D::Vector3> torch_positions;
    std::vector<Urho3D::Vector3> flag_positions;
    std::vector<level_rock_spawn> rock_spawns;
    std::vector<level_static_model> static_models;    ///< static level geometry like terrain and buildings. Will all get a triangle-mesh collider.
    Urho3D::Vector3 player_pos;
    Urho3D::String sound_name;

    level(){}
    /// loads a level from an XML file
    level(std::string filename);
    //void save();                    // saves a level to an XML file, not implemented but could be useful for an editor
};

/// The game state handling playing a level.
class gs_playing : public game_state
{
public:
    Urho3D::Text* text_;
    double timer_playing=0;
    float goal_time=0;
    std::vector<Urho3D::Node*> flag_nodes;
    float cam_distance=14;
    float camera_yaw=20;
    float camera_pitch=20;
    bool rocks_spawned=false;
    bool camera_first_person=false;
    static std::string last_level_filename;
    std::unique_ptr<player> player_;
    float level_min_height=999999;
    Urho3D::SoundSource* sound_source_wind;
    Urho3D::Sound* sound_wind;
    level current_level;

    gs_playing(std::string level_filename);
    void update(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleKeyDown(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void spawn_torch(Urho3D::Vector3 pos);

    virtual const Urho3D::String& GetTypeName() const {static Urho3D::String name("gs_playing");return name;}   // this could be correct
};

#endif // GS_PLAYING_H
