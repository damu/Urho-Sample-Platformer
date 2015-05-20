#ifndef PLAYER_H
#define PLAYER_H

#include "gs_playing.h"

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
    Urho3D::Node* node;
    Urho3D::Node* node_model;
    Urho3D::Node* node_light;
    Urho3D::RigidBody* body;
    gs_playing* gs;

    player(Urho3D::Vector3 pos,gs_playing* gs);
    void update(Urho3D::Input* input,float timeStep);
};

#endif // PLAYER_H
