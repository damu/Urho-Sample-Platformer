#include "player.h"

using namespace Urho3D;

player::player(Vector3 pos,game_state* gs)
{
    node=globals::instance()->scene->CreateChild("Player");
    gs->nodes.push_back(node);
    node_model=globals::instance()->scene->CreateChild();
    gs->nodes.push_back(node_model);

    AnimatedModel* boxObject=node_model->CreateComponent<AnimatedModel>();
    set_model(boxObject,globals::instance()->cache,"Data/Models/robot");
    boxObject->SetCastShadows(true);

    body=node->CreateComponent<RigidBody>();
    body->SetPosition(pos);
    pos_last=body->GetPosition();
    body->SetCollisionLayer(1);
    body->SetMass(80.0);
    body->SetLinearDamping(0.0f);
    body->SetAngularDamping(0.98f);
    body->SetAngularFactor(Vector3(0,1,0));
    body->SetFriction(0.8);
    CollisionShape* shape=node->CreateComponent<CollisionShape>();
    shape->SetCapsule(1.3,2,Vector3(0,1.05,0));

    {
        Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_stand.ani");
        as_stand=boxObject->AddAnimationState(ani);
        as_stand->SetWeight(1.0f);
        as_stand->SetLooped(true);
    }
    /*{
        Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_walk.ani");
        player_walk=boxObject->AddAnimationState(ani);
        player_walk->SetWeight(0.0f);
        player_walk->SetLooped(true);
    }*/
    {
        Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_run.ani");
        as_run=boxObject->AddAnimationState(ani);
        as_run->SetWeight(0.0f);
        as_run->SetLooped(true);
    }
    {
        Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_jump.ani");
        as_jump=boxObject->AddAnimationState(ani);
        as_jump->SetWeight(0.0f);
        as_jump->SetLooped(true);
    }
    {
        Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_reversing.ani");
        as_reversing=boxObject->AddAnimationState(ani);
        as_reversing->SetWeight(0.0f);
        as_reversing->SetLooped(true);
    }

    {
        auto n_particle=node_model->GetChild("torso",true)->CreateChild();
        n_particle->Translate(Vector3(0,0.224,-0.224));
        n_particle->Pitch(90);
        n_particle->Roll(180);
        emitter_back=n_particle->CreateComponent<ParticleEmitter>();
        emitter_back->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
        emitter_back->SetEmitting(false);
    }
    {
        auto n_particle=node_model->GetChild("torso",true)->CreateChild();
        n_particle->Translate(Vector3(0,0.224,0.10085));
        n_particle->Pitch(90);
        emitter_front=n_particle->CreateComponent<ParticleEmitter>();
        emitter_front->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
        emitter_front->SetEmitting(false);
    }
    {
        auto n_particle=node_model->GetChild("torso",true)->CreateChild();
        n_particle->Translate(Vector3(0.14778,0.224,-0.06949));
        n_particle->Pitch(90);
        n_particle->Roll(270);
        emitter_left=n_particle->CreateComponent<ParticleEmitter>();
        emitter_left->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
        emitter_left->SetEmitting(false);
    }
    {
        auto n_particle=node_model->GetChild("torso",true)->CreateChild();
        n_particle->Translate(Vector3(-0.14778,0.224,-0.06949));
        n_particle->Pitch(90);
        n_particle->Roll(90);
        emitter_right=n_particle->CreateComponent<ParticleEmitter>();
        emitter_right->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
        emitter_right->SetEmitting(false);
    }
    // feet thruster (for jumping)
    {
        auto n_particle=node_model->GetChild("foot.L",true)->CreateChild();
        n_particle->SetDirection(Vector3::DOWN);  // the effect is kinda reversed
        //n_particle->Translate(Vector3(-0.14778,0.224,-0.06949));
        //n_particle->Pitch(90);
        //n_particle->Roll(90);
        emitter_foot_left=n_particle->CreateComponent<ParticleEmitter>();
        emitter_foot_left->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster_feet.xml"));
        emitter_foot_left->SetEmitting(false);
    }
    {
        auto n_particle=node_model->GetChild("foot.R",true)->CreateChild();
        n_particle->SetDirection(Vector3::DOWN);
        //n_particle->Translate(Vector3(-0.14778,0.224,-0.06949));
        //n_particle->Pitch(90);
        //n_particle->Roll(90);
        emitter_foot_right=n_particle->CreateComponent<ParticleEmitter>();
        emitter_foot_right->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster_feet.xml"));
        emitter_foot_right->SetEmitting(false);
    }

    {
        node_light=node_model->GetChild("head",true)->CreateChild();
        node_light->Translate(Vector3(0,0.35,-0.35));
        node_light->Pitch(180);
        Light* light=node_light->CreateComponent<Light>();
        light->SetLightType(LIGHT_SPOT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(0.5,0.7,1.0,1.0)*3);
        light->SetCastShadows(true);
        light->SetFov(100);
    }

    {
        sound_step1=globals::instance()->cache->GetResource<Sound>("Sounds/step1.ogg");
        sound_step2=globals::instance()->cache->GetResource<Sound>("Sounds/step2.ogg");
        sound_source1=node_model->CreateComponent<SoundSource3D>();
        sound_source1->SetNearDistance(1);
        sound_source1->SetFarDistance(55);
        sound_source1->SetSoundType(SOUND_EFFECT);
        sound_source2=node_model->CreateComponent<SoundSource3D>();
        sound_source2->SetNearDistance(1);
        sound_source2->SetFarDistance(55);
        sound_source2->SetSoundType(SOUND_EFFECT);

        sound_steam=globals::instance()->cache->GetResource<Sound>("Sounds/steam.ogg");
        sound_steam->SetLooped(true);
        sound_source_steam=node_model->CreateComponent<SoundSource3D>();
        sound_source_steam->SetNearDistance(1);
        sound_source_steam->SetFarDistance(55);
        sound_source_steam->SetSoundType(SOUND_EFFECT);

        sound_flag=globals::instance()->cache->GetResource<Sound>("Sounds/littlerobotsoundfactory__jingle-win-synth-04.ogg");
        sound_source_flag=node_model->CreateComponent<SoundSource3D>();
        sound_source_flag->SetNearDistance(1);
        sound_source_flag->SetFarDistance(55);
        sound_source_flag->SetSoundType(SOUND_EFFECT);
    }
}

void player::update(Input* input,float timeStep)
{
    emitter_right->SetEmitting(false);
    emitter_left->SetEmitting(false);
    emitter_front->SetEmitting(false);
    emitter_back->SetEmitting(false);
    emitter_foot_left->SetEmitting(false);
    emitter_foot_right->SetEmitting(false);

    Node* node_camera=globals::instance()->camera->GetNode();

    {
        Vector3 moveDir=Vector3::ZERO;
        Vector3 moveDir_global=Vector3::ZERO;

        if(input->GetKeyDown('D'))
            moveDir+=Vector3::RIGHT*1;
        if(input->GetKeyDown('A'))
            moveDir-=Vector3::RIGHT*1;
        if(input->GetKeyDown('W'))
            moveDir+=Vector3::FORWARD*1;
        if(input->GetKeyDown('S'))
            moveDir-=Vector3::FORWARD*1;

        if(moveDir.Length()>0.1)
            body->SetFriction(0.4);
        else
            body->SetFriction(2.0);

        if(moveDir.Length()>0.5)
            moveDir.Normalize();

        Vector3 vel=body->GetLinearVelocity()*Vector3(1,0,1);
        Quaternion rot=body->GetRotation();

        static bool on_floor;
        Vector3 moveDir_world;
        {
            static bool at_wall;
            static int jumping=0; // 0 = not jumping, 1 = jumping, 2 =
            on_floor=false;
            at_wall=false;

            float height=0;
            PhysicsRaycastResult result;
            Ray ray(node->GetPosition()+Vector3(0,1.0,0),Vector3::DOWN);
            globals::instance()->physical_world->SphereCast(result,ray,0.2,2,2);
            if(result.distance_<=2)
                on_floor=true;
            else
            {
                globals::instance()->physical_world->SphereCast(result,ray,1.7,2,2);
                if(result.distance_<=2)
                    at_wall=true;
            }
            if(!on_floor)
                moveDir*=0.35;

            as_stand->AddTime(timeStep/2);
            //player_walk->AddTime(timeStep*vel.Length()/1.5);
            as_run->AddTime(timeStep*vel.Length()/3);
            as_jump->AddTime(timeStep);
            as_reversing->AddTime(timeStep);
            //player_stand->SetWeight(1.0-Clamp(vel.Length()/2,0.0,1.0));
            as_stand->SetWeight(1.0);
            //player_walk->SetWeight(Clamp(vel.Length()/2,0.0,1.0));
            as_run->SetWeight(Clamp((vel.Length()-2)/2,0.0,1.0));   // maybe this should be done differently, but works for this game
            if(!on_floor)
                as_jump->SetWeight(as_jump->GetWeight()+timeStep*5);
            else
            {
                as_jump->SetWeight(0.0);

                static bool sound_step1_not_played=true;    // not player this animation cycle
                static bool sound_step2_not_played=true;
                if(as_run->GetTime()>0.1&&sound_step1_not_played&&as_reversing->GetWeight()<0.5)
                {
                    sound_source1->Play(sound_step1);
                    sound_step1_not_played=false;
                }
                if(as_run->GetTime()>0.6&&sound_step2_not_played&&as_reversing->GetWeight()<0.5)
                {
                    sound_source2->Play(sound_step2);
                    sound_step2_not_played=false;
                }
                if(as_run->GetTime()<0.1)
                {
                    sound_step1_not_played=true;
                    sound_step2_not_played=true;
                }
            }

            if(input->GetKeyDown(KEY_SPACE)&&jumping==false&&(on_floor||at_wall))   // walljump
            {
                jumping=1;  // start jumping
                if(at_wall)
                {
                    auto v=result.normal_*Vector3(1,0,1)*1.7+vel*Vector3(1,0,1)*0.2;    // the result.normal vector is sometimes very weird if
                    v.Normalize();                                                      // there are multiple faces nearby
                    body->SetLinearVelocity(Vector3(v.x_*10,0,v.z_*10));
                    vel=body->GetLinearVelocity()*Vector3(1,0,1);
                }
            }
            else if(!input->GetKeyDown(KEY_SPACE))
                jumping=0;

            static float jump_force_applied=0;
            static const float max_jump_force_applied=700;
            moveDir_world=node->GetWorldRotation()*moveDir;

            if(moveDir_world.Angle(vel)>90&&vel.Length()>3&&on_floor)   // indicate if direction change jump / side sommersault possible
                as_reversing->SetWeight(as_reversing->GetWeight()+timeStep*10);
            else
                as_reversing->SetWeight(0.0);

            if(jumping==1&&jump_force_applied<max_jump_force_applied)   // jump higher if we are jumping and the limit has not been reached
            {
                if(jump_force_applied>max_jump_force_applied)
                {
                    // do nothing if max jump force reached
                }
                else if(jump_force_applied+timeStep*4000>max_jump_force_applied)
                {
                    // I want to limit the jump height more exactly by limiting the force pumped into it and applieng the remaining rest here. Doesn't fully work yet.
                    //float f=0;//(max_jump_force_applied-jump_force_applied)*timeStep*2000;
                    //moveDir+=Vector3::UP*2*f;
                    //moveDir_global=result.normal_*1*f;
                    jump_force_applied+=timeStep*5000;
                }
                else
                {
                    float f=1;
                    if(moveDir_world.Angle(vel)>90&&vel.Length()>3&&on_floor) // direction change jump / side sommersault
                    {
                        f=1.3;
                        body->SetLinearVelocity(Vector3(moveDir_world.x_*10,body->GetLinearVelocity().y_,moveDir_world.z_*10));
                        vel=body->GetLinearVelocity()*Vector3(1,0,1);
                    }
                    moveDir+=Vector3::UP*2*f;
                    moveDir_global=result.normal_*1*f;
                    jump_force_applied+=timeStep*5000;
                    emitter_foot_left->SetEmitting(true);
                    emitter_foot_right->SetEmitting(true);
                    if(!sound_source_steam->IsPlaying())
                        sound_source_steam->Play(sound_steam);
                }
            }
            if(jumping!=1)
                jump_force_applied=0;

            if(!emitter_foot_left->IsEmitting()&&sound_source_steam->IsPlaying())
                sound_source_steam->Stop();
        }

        Quaternion quat;
        quat.FromLookRotation(node_camera->GetDirection()*Vector3(1,0,1),Vector3::UP);
        body->SetRotation(quat);
        float speed_old=vel.Length();
        vel+=rot*moveDir*timeStep*4000/body->GetMass();
        float speed_new=vel.Length();
        if(on_floor)
        if(speed_new>20&&speed_new>speed_old)   // over limit. Don't increase speed further but make direction change possible.
            vel=vel.Normalized()*speed_old;
        body->SetLinearVelocity(Vector3(vel.x_,body->GetLinearVelocity().y_+(rot*moveDir*timeStep*6000/body->GetMass()).y_,vel.z_));
        body->ApplyImpulse(moveDir_global*timeStep*5000);

        {
            auto vec_rot=body->GetLinearVelocity()*Vector3(1,0,1);
            float s=vec_rot.Length();
            vec_rot.Normalize();
            float yaw=asin(vec_rot.x_)*180/M_PI;
            if(vec_rot.z_<0)
                yaw=180-yaw;
            node_model->SetPosition(node->GetPosition());
            if(s>1&&!camera_first_person)
            {
                node_model->SetDirection(Vector3::FORWARD);
                node_model->Yaw(yaw);
            }

            // this was for the four thrusters around the player which should explain the movement changes in air. But I couldn't figure out the correct calculations.
            //if(!on_floor)
            /*{
                yaw+=90;
                yaw-=camera_yaw;
                if(yaw<0)
                    yaw+=360;

                Vector3 v(moveDir_world.Length()*-Sin(yaw),0,moveDir_world.Length()*Cos(yaw));
                if(yaw>280&&yaw<80)         // around 0
                    player_emitter_right->SetEmitting(true);
                else if(yaw>100&&yaw<260)   // around 180
                    player_emitter_left->SetEmitting(true);
                if(yaw>10&&yaw<170)         // around 90
                    player_emitter_back->SetEmitting(true);
                else if(yaw>190&&yaw<350)   // around 270
                    player_emitter_front->SetEmitting(true);
str.append(std::to_string(yaw));
str.append(std::to_string(camera_yaw));
String s(str.c_str(),str.size());
text_->SetText(s);
            }*/
        }

        {   // physic raycast to avoid the player glitching through stuff when moving very fast
            Vector3 player_pos=body->GetPosition()+Vector3(0,1,0);
            PhysicsRaycastResult result;
            Ray ray(pos_last,player_pos-pos_last);
            float l=(player_pos-pos_last).Length();
            if(l>0.5)
            {
                globals::instance()->physical_world->SphereCast(result,ray,0.2,l,2);
                if(result.distance_<=l)
                    body->SetPosition(pos_last);
                pos_last=body->GetPosition()+Vector3(0,1,0);
            }
        }
    }

    IntVector2 mouseMove(0,0);
    if(!input->IsMouseVisible())
        mouseMove=input->GetMouseMove();

    camera_yaw+=mouseMove.x_*0.1;
    if(camera_yaw<0)
        camera_yaw+=360;
    if(camera_yaw>=360)
        camera_yaw-=360;
    camera_pitch+=mouseMove.y_*0.1;
    camera_pitch=Clamp(camera_pitch,-85.0,85.0);
    if(!camera_first_person)
    {
        camera_distance-=input->GetMouseMoveWheel();
        camera_distance=Clamp(camera_distance,2.0,50.0);

        node_camera->SetPosition(node->GetPosition());
        node_camera->SetDirection(Vector3::FORWARD);
        node_camera->Yaw(camera_yaw);
        node_camera->Translate(Vector3(0,1.6,0));
        node_camera->Pitch(camera_pitch);

        PhysicsRaycastResult result;
        Ray ray(node_camera->GetPosition(),-node_camera->GetDirection());
        globals::instance()->physical_world->SphereCast(result,ray,0.2,camera_distance,2);
        if(result.distance_<=camera_distance)
            node_camera->Translate(Vector3(0,0,-result.distance_+0.1));
        else
            node_camera->Translate(Vector3(0,0,-camera_distance));
    }
    else
    {
        node_camera->SetPosition(node_light->GetWorldPosition());
        node_camera->SetDirection(Vector3::FORWARD);
        node_camera->Yaw(camera_yaw);
        node_camera->Pitch(camera_pitch);
        node_camera->Translate(Vector3(0,0,0.1));
        node_model->SetDirection(Vector3::FORWARD);
        node_model->Yaw(camera_yaw);
    }
}
