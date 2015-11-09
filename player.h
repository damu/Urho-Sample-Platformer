#ifndef PLAYER_H
#define PLAYER_H

#include "gs_playing.h"

/// \brief Creates and manages the player and the camera.
class player
{
public:
    Urho3D::Vector3 pos_last;
    Urho3D::AnimationState* as_stand;
    //Urho3D::AnimationState* as_walk;
    Urho3D::AnimationState* as_run;
    Urho3D::AnimationState* as_jump;
    Urho3D::AnimationState* as_reversing;
    Urho3D::ParticleEmitter* emitter_back;
    Urho3D::ParticleEmitter* emitter_front;
    Urho3D::ParticleEmitter* emitter_left;
    Urho3D::ParticleEmitter* emitter_right;
    Urho3D::ParticleEmitter* emitter_foot_left;
    Urho3D::ParticleEmitter* emitter_foot_right;
    Urho3D::Node* node;
    Urho3D::Node* node_model;
    Urho3D::Node* node_light;
    Urho3D::RigidBody* body;
    Urho3D::SoundSource3D* sound_source1;
    Urho3D::SoundSource3D* sound_source2;
    Urho3D::SoundSource3D* sound_source_steam;
    Urho3D::SoundSource3D* sound_source_flag;
    Urho3D::Sound* sound_step1;
    Urho3D::Sound* sound_step2;
    Urho3D::Sound* sound_steam;
    Urho3D::Sound* sound_flag;
    float camera_distance=14;
    float camera_yaw=20;
    float camera_pitch=20;
    bool camera_first_person=false;

    /// \brief The game state is given to add the player nodes to the node clean-up list of the game state.
    player(Urho3D::Vector3 pos,game_state* gs);
    /// \brief Called each frame.
    void update(Urho3D::Input* input,float timeStep);
};

#endif // PLAYER_H
