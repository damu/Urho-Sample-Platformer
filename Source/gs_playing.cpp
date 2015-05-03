#include "gs_playing.h"

#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>

#include "gs_pause.h"

using namespace Urho3D;

gs_playing::gs_playing() : game_state()
{
    text_=new Text(context_);
    gui_elements.push_back(text_);
    text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
    text_->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
    text_->SetColor(Color(.6,.3,.3));
    text_->SetHorizontalAlignment(HA_CENTER);
    text_->SetVerticalAlignment(VA_TOP);
    GetSubsystem<UI>()->GetRoot()->AddChild(text_);
    Button* button=new Button(context_);
    gui_elements.push_back(button);
    GetSubsystem<UI>()->GetRoot()->AddChild(button);
    button->SetName("Button Quit");
    button->SetStyle("Button");
    button->SetSize(32,32);
    button->SetPosition(16,16);

    GetSubsystem<Input>()->SetMouseVisible(false);
    GetSubsystem<Input>()->SetMouseGrabbed(true);

    SubscribeToEvent(E_UPDATE,HANDLER(gs_playing,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_playing,HandleKeyDown));

    {
        Node* boxNode_=globals::instance()->scene->CreateChild("Box");
        nodes.push_back(boxNode_);
        boxNode_->SetPosition(Vector3(0,0,-10));
        StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/level_1.mdl"));
        boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/level_1_mushroom.xml"));
        boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/level_1_terrain.xml"));
        boxObject->SetMaterial(2,globals::instance()->cache->GetResource<Material>("Materials/level_1_white_walls.xml"));
        boxObject->SetCastShadows(true);
        boxObject->SetOccludee(true);
        boxObject->SetOccluder(true);

        RigidBody* body=boxNode_->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
        CollisionShape* shape=boxNode_->CreateComponent<CollisionShape>();
        shape->SetTriangleMesh(globals::instance()->cache->GetResource<Model>("Models/level_1.mdl"));

        body->GetPhysicsWorld()->SetGravity(Vector3(0,-9.81*4,0));
    }

    {
        node_player=globals::instance()->scene->CreateChild("Player");
        nodes.push_back(node_player);
        node_player_model=globals::instance()->scene->CreateChild();
        nodes.push_back(node_player_model);
        StaticModel* boxObject=node_player_model->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/robot.mdl"));
        boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/robot_dark_grey.xml"));
        boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/robot_white.xml"));
        boxObject->SetMaterial(2,globals::instance()->cache->GetResource<Material>("Materials/robot_dark.xml"));
        boxObject->SetMaterial(3,globals::instance()->cache->GetResource<Material>("Materials/robot_light.xml"));
        boxObject->SetMaterial(4,globals::instance()->cache->GetResource<Material>("Materials/robot_black.xml"));
        boxObject->SetMaterial(5,globals::instance()->cache->GetResource<Material>("Materials/robot_eyes.xml"));
        boxObject->SetCastShadows(true);

        body_player=node_player->CreateComponent<RigidBody>();
        body_player->SetPosition(Vector3(4,10,-5));
        body_player->SetCollisionLayer(1);
        body_player->SetMass(80.0);
        body_player->SetLinearDamping(0.0f);
        body_player->SetAngularDamping(0.98f);
        body_player->SetAngularFactor(Vector3(0,1,0));
        body_player->SetFriction(0.8);
        CollisionShape* shape=node_player->CreateComponent<CollisionShape>();
        shape->SetCapsule(1,2,Vector3(0,1.05,0));
    }

    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00000025f,1.0f));
        light->SetShadowCascade(CascadeParameters(20.0f,60.0f,180.0f,560.0f,100.0f,100.0f));
        light->SetShadowResolution(1.0);
        light->SetBrightness(1.2);
        light->SetColor(Color(1,.8,.7,1));
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(45);
        lightNode->Roll(30);
        lightNode->Pitch(60);
    }

    {   // "load" flags
        flag_positions.emplace_back(20.35,14,-47.6);
        flag_positions.emplace_back(-9.9,14,-47.7);
        flag_positions.emplace_back(-0.25,-5.5,-195.6);
        flag_positions.emplace_back(18.2,21.6,-5.1);
        flag_positions.emplace_back(28.7,33.9,82.6);
        flag_positions.emplace_back(110,38.7,57.1);
        flag_positions.emplace_back(-238,37,-125);

        for(auto p:flag_positions)
        {
            Node* n=globals::instance()->scene->CreateChild("Flag");
            nodes.push_back(n);
            n->SetPosition(Vector3(p.x_,p.y_,p.z_));
            StaticModel* boxObject=n->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/flag.mdl"));
            boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/flag_pole.xml"));
            boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/flag_cloth.xml"));
            boxObject->SetCastShadows(true);
            flag_nodes.push_back(n);
        }
    }

    // spawn one rock and remove it to cache the collider mesh (to avoid a ~1 second lag when spawning the first rock during the game)
    {
        auto node_stone=globals::instance()->scene->CreateChild("Stone");
        StaticModel* boxObject=node_stone->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
        boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/rock.xml"));
        boxObject->SetCastShadows(true);
        float s=1.0+Random(3.0f);
        node_stone->SetScale(s);

        PhysicsRaycastResult result;
        Vector3 pos(-120-Random(100.0f),100,-70-Random(100.0f));
        Ray ray(pos,Vector3(0,-1,0));
        body_player->GetPhysicsWorld()->SphereCast(result,ray,2,100);
        if(result.distance_<=1000)
            pos=result.position_+Vector3(0,5,0);

        auto body_stone=node_stone->CreateComponent<RigidBody>();
        body_stone->SetPosition(pos);
        body_stone->SetCollisionLayer(2);
        body_stone->SetMass(50.0*s*s);
        body_stone->SetLinearDamping(0.2f);
        body_stone->SetAngularDamping(0.2f);
        //body_stone->SetAngularFactor(Vector3(0,1,0));
        body_stone->SetFriction(0.6);
        CollisionShape* shape=node_stone->CreateComponent<CollisionShape>();
        //shape->SetCapsule(1,1.2);
        shape->SetConvexHull(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
        node_stone->Remove();
    }

    timer_playing=0;
}

void gs_playing::update(StringHash eventType,VariantMap& eventData)
{
    if(globals::instance()->game_state_.size()>1)
        return;

    Input* input=GetSubsystem<Input>();
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    framecount_++;
    time_+=timeStep;
    timer_playing+=timeStep;

    if(!rocks_spawned&&(node_player->GetPosition()-Vector3(-200,37,-125)).Length()<150)
    {
        rocks_spawned=true;
        for(int i=0;i<150;i++)
        {
            auto node_stone=globals::instance()->scene->CreateChild("Stone");
            nodes.push_back(node_stone);
            StaticModel* boxObject=node_stone->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
            boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/rock.xml"));
            boxObject->SetCastShadows(true);
            float s=1.0+Random(3.0f);
            node_stone->SetScale(s);
            boxObject->SetOccludee(true);
            boxObject->SetDrawDistance(200);
            boxObject->SetShadowDistance(200);

            PhysicsRaycastResult result;
            Vector3 pos(-120-Random(100.0f),100,-70-Random(100.0f));
            Ray ray(pos,Vector3(0,-1,0));
            body_player->GetPhysicsWorld()->SphereCast(result,ray,2,100);
            if(result.distance_<=1000)
                pos=result.position_+Vector3(0,5,0);

            auto body_stone=node_stone->CreateComponent<RigidBody>();
            body_stone->SetPosition(pos);
            body_stone->SetCollisionLayer(2);
            body_stone->SetMass(50.0*s*s);
            body_stone->SetLinearDamping(0.2f);
            body_stone->SetAngularDamping(0.2f);
            //body_stone->SetAngularFactor(Vector3(0,1,0));
            body_stone->SetFriction(0.6);
            CollisionShape* shape=node_stone->CreateComponent<CollisionShape>();
            //shape->SetCapsule(1,1.2);
            shape->SetConvexHull(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
        }
    }

    {
        static std::string str_inner;
        if(time_ >=1)
        {
            str_inner="";
            str_inner.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
            str_inner.append(std::to_string(framecount_));
            str_inner.append(" frames in ");
            str_inner.append(std::to_string(time_));
            str_inner.append(" seconds = ");
            str_inner.append(std::to_string((float)framecount_ / time_));
            str_inner.append(" fps\nLevel Time: ");
            framecount_=0;
            time_=0;
        }
        std::string str;
        str.append(str_inner);

        if(goal_time>0)
            str.append(std::to_string(goal_time));
        else
        {
            str.append(std::to_string(timer_playing));
            if(!flag_nodes.size())
                goal_time=timer_playing;
        }

        str.append("s\nPosition: ");
        str.append(std::to_string(node_player->GetPosition().x_));
        str.append(", ");
        str.append(std::to_string(node_player->GetPosition().y_));
        str.append(", ");
        str.append(std::to_string(node_player->GetPosition().z_));
        str.append("\nRemaining Flags: ");
        str.append(std::to_string(flag_nodes.size()));
        str.append("/");
        str.append(std::to_string(flag_positions.size()));
        if(goal_time>0)
            str.append("\nFinished!");

        String s(str.c_str(),str.size());
        text_->SetText(s);
    }

    IntVector2 mouseMove(0,0);
    if(!input->IsMouseVisible())
        mouseMove=input->GetMouseMove();

    Node* node_camera=globals::instance()->camera->GetNode();
    {
        cam_distance-=input->GetMouseMoveWheel();
        cam_distance=Clamp(cam_distance,2.0,50.0);

        node_camera->SetPosition(node_player->GetPosition());
        node_camera->SetDirection(Vector3::FORWARD);
        camera_yaw+=mouseMove.x_*0.1;
        node_camera->Yaw(camera_yaw);
        camera_pitch+=mouseMove.y_*0.1;
        camera_pitch=Clamp(camera_pitch,-85.0,85.0);
        node_camera->Translate(Vector3(0,1,0));
        node_camera->Pitch(camera_pitch);

        PhysicsRaycastResult result;
        Ray ray(node_camera->GetPosition(),-node_camera->GetDirection());
        body_player->GetPhysicsWorld()->SphereCast(result,ray,0.2,cam_distance,2);
        if(result.distance_<=cam_distance)
            node_camera->Translate(Vector3(0,0,-result.distance_+0.1));
        else
            node_camera->Translate(Vector3(0,0,-cam_distance));
    }
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
            body_player->SetFriction(0.4);
        else
            body_player->SetFriction(2.0);

        if(moveDir.Length()>0.5)
            moveDir.Normalize();

        Vector3 vel=body_player->GetLinearVelocity()*Vector3(1,0,1);
        Quaternion rot=node_player->GetRotation();

        {
            static bool on_floor;
            static bool at_wall;
            static int jumping=0; // 0 = not jumping, 1 = jumping, 2 =
            on_floor=false;
            at_wall=false;

            float height=0;
            PhysicsRaycastResult result;
            Ray ray(node_player->GetPosition()+Vector3(0,1.0,0),Vector3::DOWN);
            body_player->GetPhysicsWorld()->SphereCast(result,ray,0.2,2,2);
            if(result.distance_<=2)
                on_floor=true;
            else
            {
                body_player->GetPhysicsWorld()->SphereCast(result,ray,1.5,2,2);
                if(result.distance_<=2)
                    at_wall=true;
            }
            if(!on_floor)
                moveDir*=0.35;

            if(input->GetKeyDown(KEY_SPACE)&&jumping==false&&(on_floor||at_wall))   // walljump
            {
                jumping=1;  // start jumping
                if(at_wall)
                {
                    auto v=result.normal_*Vector3(1,0,1)*1.7+vel*Vector3(1,0,1)*0.2;    // the result.normal vector is sometimes very weird if
                    v.Normalize();                                                      // there are multiple faces near
                    body_player->SetLinearVelocity(Vector3(v.x_*10,0,v.z_*10));
                    vel=body_player->GetLinearVelocity()*Vector3(1,0,1);
                }
            }
            else if(!input->GetKeyDown(KEY_SPACE))
                jumping=0;

            static float jump_force_applied=0;
            static const float max_jump_force_applied=700;
            Vector3 moveDir_world=node_player->GetWorldRotation()*moveDir;
            if(jumping==1&&jump_force_applied<max_jump_force_applied)   // jump higher if we are jumping and the limit has not been reached
            {
                if(jump_force_applied>max_jump_force_applied)
                {
                    // do nothing if max jump force reached
                }
                else if(jump_force_applied+timeStep*4000>max_jump_force_applied)
                {
                    // I want to limit the jump height more exactly by limiting the force pumped into it and applieng the remaining rest here. Doesn't fully work yet.
                    float f=0;//(max_jump_force_applied-jump_force_applied)*timeStep*2000;
                    moveDir+=Vector3::UP*2*f;
                    moveDir_global=result.normal_*1*f;
                    jump_force_applied+=timeStep*5000;
                }
                else
                {
                    float f=1;
                    if(moveDir_world.Angle(vel)>90&&vel.Length()>3&&on_floor) // direction change jump / side sommersault
                    {
                        f=1.3;
                        body_player->SetLinearVelocity(Vector3(moveDir_world.x_*10,body_player->GetLinearVelocity().y_,moveDir_world.z_*10));
                        vel=body_player->GetLinearVelocity()*Vector3(1,0,1);
                    }
                    moveDir+=Vector3::UP*2*f;
                    moveDir_global=result.normal_*1*f;
                    jump_force_applied+=timeStep*5000;
                }
            }
            if(jumping!=1)
                jump_force_applied=0;
        }

        Quaternion quat;
        quat.FromLookRotation(node_camera->GetDirection()*Vector3(1,0,1),Vector3::UP);
        body_player->SetRotation(quat);
        float speed_old=vel.Length();
        vel+=rot*moveDir*timeStep*4000/body_player->GetMass();
        float speed_new=vel.Length();
        if(speed_new>20&&speed_new>speed_old)   // over limit. Don't increase speed further but make direction change possible.
        {
            vel=vel.Normalized()*speed_old;
/*            std::string s;
            s+=std::to_string(speed_old);
            s+=std::to_string(speed_new);
            s+=std::to_string(vel.Length());
            LOGINFO(String(s.data(),s.size()));*/
        }
        body_player->SetLinearVelocity(Vector3(vel.x_,body_player->GetLinearVelocity().y_+(rot*moveDir*timeStep*6000/body_player->GetMass()).y_,vel.z_));
        body_player->ApplyImpulse(moveDir_global*timeStep*5000);

        auto vec_rot=body_player->GetLinearVelocity()*Vector3(1,0,1);
        float s=vec_rot.Length();
        vec_rot.Normalize();
        float yaw=asin(vec_rot.x_)*180/M_PI;
        if(vec_rot.z_<0)
            yaw=-yaw-180;
        node_player_model->SetPosition(node_player->GetPosition());
        if(s>1)
        {
            node_player_model->SetDirection(Vector3::FORWARD);
            node_player_model->Yaw(yaw);
        }

        {   // physic raycast to avoid the player glitching through stuff when moving very fast
            Vector3 player_pos=body_player->GetPosition()+Vector3(0,1,0);
            static Vector3 player_pos_last=player_pos;
            PhysicsRaycastResult result;
            Ray ray(player_pos_last,player_pos-player_pos_last);
            float l=(player_pos-player_pos_last).Length();
            if(l>0.5)
            {
                body_player->GetPhysicsWorld()->SphereCast(result,ray,0.2,l,2);
                if(result.distance_<=l)
                    body_player->SetPosition(player_pos_last);
                player_pos_last=body_player->GetPosition()+Vector3(0,1,0);
            }
        }
    }

    Vector3 player_pos=node_player->GetPosition();
    for(int i=0;i<flag_nodes.size();i++)
    {
        auto n=flag_nodes[i];
        n->Yaw(64*timeStep);
        if((player_pos-n->GetPosition()).Length()<2)
        {
            flag_nodes.erase(flag_nodes.begin()+i);
            n->Remove();
            for(int j=0;j<nodes.size();j++) // theoretically it should be better to use a set instead of a vector when
                if(nodes[j]==n)             // needing to search for stuff, but benchmarks show otherwise and this is not done that often
                {
                    nodes.erase(nodes.begin()+j);
                    break;
                }

            break;
        }
    }
}

void gs_playing::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    if(globals::instance()->game_state_.size()>1)
        return;
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->game_state_.emplace_back(new gs_pause);

    if(key==KEY_L)
    {
        Node* node=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(node);
        Vector3 pos(node_player->GetPosition()+Vector3(2,1.9,0));

        PhysicsRaycastResult result;
        Ray ray(pos,Vector3(0,-1,0));
        body_player->GetPhysicsWorld()->SphereCast(result,ray,0.4,10);
        if(result.distance_<=10)
            pos=result.position_+Vector3(0,0.2,0);
        node->SetPosition(pos);

        StaticModel* boxObject=node->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/torch.mdl"));
        boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/torch_metal.xml"));
        boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/torch_wood.xml"));
        boxObject->SetCastShadows(true);
        boxObject->SetOccludee(true);
        boxObject->SetShadowDistance(200);
        boxObject->SetDrawDistance(200);

        auto lightNode=node->CreateChild();
        lightNode->Translate(Vector3(0,2,0));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(1.0,.6,.4,1.0));
        light->SetCastShadows(true);
        light->SetShadowDistance(200);
        light->SetDrawDistance(200);

        auto body_stone=node->CreateComponent<RigidBody>();
        body_stone->SetCollisionLayer(2);
        body_stone->SetMass(50.0);
        body_stone->SetLinearDamping(0.2f);
        body_stone->SetAngularDamping(0.2f);
        body_stone->SetFriction(0.6);
        CollisionShape* shape=node->CreateComponent<CollisionShape>();
        //shape->SetCapsule(1,1.2);
        shape->SetBox(Vector3(0.8,2,0.8),Vector3(0,1,0));

        auto n_particle=node->CreateChild();
        n_particle->Translate(Vector3(0,1.7,0));
        ParticleEmitter* emitter=n_particle->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/torch_fire.xml"));
        emitter=n_particle->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/torch_smoke.xml"));

        auto sound_torch=globals::instance()->cache->GetResource<Sound>("Sounds/torch.ogg");
        sound_torch->SetLooped(true);
        auto sound_torch_source=n_particle->CreateComponent<SoundSource3D>();
        sound_torch_source->SetNearDistance(1);
        sound_torch_source->SetFarDistance(50);
        sound_torch_source->SetSoundType(SOUND_EFFECT);
        sound_torch_source->Play(sound_torch);
    }
}
