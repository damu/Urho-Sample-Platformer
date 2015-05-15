#include "gs_playing.h"

#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Graphics/Terrain.h>

#include "gs_pause.h"
#include "gs_level_end.h"

using namespace Urho3D;

gs_playing::gs_playing() : game_state()
{
    // create a transparent window with some text to display things like level time, remaining flags and FPS
    {
        Window* window=new Window(context_);
        gui_elements.push_back(window);
        GetSubsystem<UI>()->GetRoot()->AddChild(window);
        window->SetStyle("Window");
        window->SetSize(700,100);
        window->SetColor(Color(.0,.15,.3,.5));
        window->SetAlignment(HA_CENTER,VA_TOP);

        text_=new Text(context_);
        text_->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
        text_->SetColor(Color(.8,.85,.9));
        text_->SetAlignment(HA_CENTER,VA_CENTER);
        window->AddChild(text_);
    }

    GetSubsystem<Input>()->SetMouseVisible(false);
    GetSubsystem<Input>()->SetMouseGrabbed(true);

    SubscribeToEvent(E_UPDATE,HANDLER(gs_playing,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_playing,HandleKeyDown));

    {
        Node* boxNode_=globals::instance()->scene->CreateChild("Box");
        nodes.push_back(boxNode_);
        boxNode_->SetPosition(Vector3(0,0,-10));
        StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
        set_model(boxObject,globals::instance()->cache,"Data/Models/level_1");
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
        AnimatedModel* boxObject=node_player_model->CreateComponent<AnimatedModel>();
        set_model(boxObject,globals::instance()->cache,"Data/Models/robot");
        boxObject->SetCastShadows(true);

        body_player=node_player->CreateComponent<RigidBody>();
        body_player->SetPosition(Vector3(4,10,-5));
        player_pos_last=body_player->GetPosition();
        body_player->SetCollisionLayer(1);
        body_player->SetMass(80.0);
        body_player->SetLinearDamping(0.0f);
        body_player->SetAngularDamping(0.98f);
        body_player->SetAngularFactor(Vector3(0,1,0));
        body_player->SetFriction(0.8);
        CollisionShape* shape=node_player->CreateComponent<CollisionShape>();
        shape->SetCapsule(1,2,Vector3(0,1.05,0));

        {
            Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_stand.ani");
            player_stand=boxObject->AddAnimationState(ani);
            player_stand->SetWeight(1.0f);
            player_stand->SetLooped(true);
        }
        /*{
            Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_walk.ani");
            player_walk=boxObject->AddAnimationState(ani);
            player_walk->SetWeight(0.0f);
            player_walk->SetLooped(true);
        }*/
        {
            Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_run.ani");
            player_run=boxObject->AddAnimationState(ani);
            player_run->SetWeight(0.0f);
            player_run->SetLooped(true);
        }
        {
            Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_jump.ani");
            player_jump=boxObject->AddAnimationState(ani);
            player_jump->SetWeight(0.0f);
            player_jump->SetLooped(true);
        }
        {
            Animation* ani=globals::instance()->cache->GetResource<Animation>("Models/robot_reversing.ani");
            player_reversing=boxObject->AddAnimationState(ani);
            player_reversing->SetWeight(0.0f);
            player_reversing->SetLooped(true);
        }

        {
            auto n_particle=node_player_model->GetChild("torso",true)->CreateChild();
            n_particle->Translate(Vector3(0,0.224,-0.224));
            n_particle->Pitch(90);
            player_emitter_back=n_particle->CreateComponent<ParticleEmitter>();
            player_emitter_back->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
            player_emitter_back->SetEmitting(false);
        }
        {
            auto n_particle=node_player_model->GetChild("torso",true)->CreateChild();
            n_particle->Translate(Vector3(0,0.224,0.10085));
            n_particle->Pitch(90);
            n_particle->Roll(180);
            player_emitter_front=n_particle->CreateComponent<ParticleEmitter>();
            player_emitter_front->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
            player_emitter_front->SetEmitting(false);
        }
        {
            auto n_particle=node_player_model->GetChild("torso",true)->CreateChild();
            n_particle->Translate(Vector3(0.14778,0.224,-0.06949));
            n_particle->Pitch(90);
            n_particle->Roll(270);
            player_emitter_left=n_particle->CreateComponent<ParticleEmitter>();
            player_emitter_left->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
            player_emitter_left->SetEmitting(false);
        }
        {
            auto n_particle=node_player_model->GetChild("torso",true)->CreateChild();
            n_particle->Translate(Vector3(-0.14778,0.224,-0.06949));
            n_particle->Pitch(90);
            n_particle->Roll(90);
            player_emitter_right=n_particle->CreateComponent<ParticleEmitter>();
            player_emitter_right->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/thruster.xml"));
            player_emitter_right->SetEmitting(false);
        }

        {
            auto lightNode=node_player_model->GetChild("head",true)->CreateChild();
            lightNode->Translate(Vector3(0,0.35,-0.35));
            lightNode->Pitch(180);
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_SPOT);
            light->SetRange(50);
            light->SetBrightness(2.0);
            light->SetColor(Color(0.5,0.7,1.0,1.0)*3);
            light->SetCastShadows(true);
            light->SetFov(100);
        }
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
        light->SetColor(Color(1.5,1.2,1,1));
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(45);
        lightNode->Roll(30);
        lightNode->Pitch(60);
    }

    {   // "load" flags
        flag_positions.emplace_back(20.35,14,-47.6);
        flag_positions.emplace_back(-9.9,14,-47.7);
        flag_positions.emplace_back(-0.25,-5.5,-195.6);
        flag_positions.emplace_back(18.2,21,-4);
        flag_positions.emplace_back(28.7,33.9,82.6);
        flag_positions.emplace_back(118,43,51);
        flag_positions.emplace_back(-242,36,-107);

        torch_positions.emplace_back(-244,36.2,-104);
        torch_positions.emplace_back(-199,28,-138);

        for(auto p:flag_positions)
        {
            Node* n=globals::instance()->scene->CreateChild("Flag");
            nodes.push_back(n);

            PhysicsRaycastResult result;
            Ray ray(p+Vector3(0,5,0),Vector3(0,-1,0));
            body_player->GetPhysicsWorld()->SphereCast(result,ray,2,10);
            if(result.distance_<=10)
                p=result.position_;

            n->SetPosition(p);
            StaticModel* boxObject=n->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/flag.mdl"));
            boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/flag_pole.xml"));
            boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/flag_cloth.xml"));
            boxObject->SetCastShadows(true);
            flag_nodes.push_back(n);
        }

        for(auto p:torch_positions)
            spawn_torch(p);
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
    if(globals::instance()->game_states.size()>1)
        return;

    Input* input=GetSubsystem<Input>();
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    timer_playing+=timeStep;

    if(!rocks_spawned&&(node_player->GetPosition()-Vector3(-200,37,-125)).Length()<150)
    {
        rocks_spawned=true;
        for(int i=0;i<100;i++)
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
            Vector3 pos(-140-Random(80.0f),40,-80-Random(70.0f));
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
        static double last_second=0;
        static double last_second_frames=1;
        static timer this_second;
        static double this_second_frames=0;
        this_second_frames++;
        if(this_second.until_now()>=1)
        {
            last_second=this_second.until_now();
            last_second_frames=this_second_frames;
            this_second.reset();
            this_second_frames=0;
        }
        std::string str;
        if(last_second!=0)
            str.append(std::to_string(last_second_frames/last_second).substr(0,6));
        str.append(" FPS   Position: ");
        str.append(std::to_string(node_player->GetPosition().x_).substr(0,6));
        str.append(", ");
        str.append(std::to_string(node_player->GetPosition().y_).substr(0,6));
        str.append(", ");
        str.append(std::to_string(node_player->GetPosition().z_).substr(0,6));
        str.append("\nLevel Time: ");

        if(goal_time>0)
            str.append(std::to_string(goal_time));
        else
        {
            str.append(std::to_string(timer_playing));
        }

        str.append("s\nRemaining Flags: ");
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
        player_emitter_right->SetEmitting(false);
        player_emitter_left->SetEmitting(false);
        player_emitter_front->SetEmitting(false);
        player_emitter_back->SetEmitting(false);

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

        static bool on_floor;
        {
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

            player_stand->AddTime(timeStep/2);
            //player_walk->AddTime(timeStep*vel.Length()/1.5);
            player_run->AddTime(timeStep*vel.Length()/3);
            player_jump->AddTime(timeStep);
            player_reversing->AddTime(timeStep);
            //player_stand->SetWeight(1.0-Clamp(vel.Length()/2,0.0,1.0));
            player_stand->SetWeight(1.0);
            //player_walk->SetWeight(Clamp(vel.Length()/2,0.0,1.0));
            player_run->SetWeight(Clamp((vel.Length()-2)/2,0.0,1.0));   // maybe this should be done differently, but works for this game
            if(!on_floor)
                player_jump->SetWeight(player_jump->GetWeight()+timeStep*5);
            else
                player_jump->SetWeight(0.0);

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

            /*{ // doesn't work yet correctly
                Vector3 v=node_player_model->GetWorldRotation()*(-moveDir_world);
                if(v.x_>0)
                    player_emitter_right->SetEmitting(true);
                else if(v.x_<0)
                    player_emitter_left->SetEmitting(true);
                if(v.y_>0)
                    player_emitter_back->SetEmitting(true);
                else if(v.y_<0)
                    player_emitter_front->SetEmitting(true);
            }*/

            if(moveDir_world.Angle(vel)>90&&vel.Length()>3&&on_floor)   // indicate if direction change jump / side sommersault possible
                player_reversing->SetWeight(player_reversing->GetWeight()+timeStep*10);
            else
                player_reversing->SetWeight(0.0);

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
        if(on_floor)
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
            for(int j=0;j<nodes.size();j++)
                if(nodes[j]==n)
                {
                    nodes.erase(nodes.begin()+j);
                    break;
                }

            if(flag_nodes.size()==0)
            {
                goal_time=timer_playing;
                auto e=new gs_level_end;
                std::string str="You collected all flags!\nNeeded time: "+std::to_string(goal_time)+"s";
                e->text_finished->SetText(str.c_str());
                globals::instance()->game_states.emplace_back(e);
            }
            break;
        }
    }
}

void gs_playing::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    if(globals::instance()->game_states.size()>1)
        return;
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->game_states.emplace_back(new gs_pause);

    if(key==KEY_L)
        spawn_torch(node_player->GetPosition()+Vector3(2,1.9,0));
}

void gs_playing::spawn_torch(Vector3 pos)
{
    Node* node=globals::instance()->scene->CreateChild("Light");
    nodes.push_back(node);

    PhysicsRaycastResult result;
    Ray ray(pos,Vector3(0,-1,0));
    body_player->GetPhysicsWorld()->SphereCast(result,ray,0.4,10);
    if(result.distance_<=10)
        pos=result.position_+Vector3(0,0.2,0);
    node->SetPosition(pos);

    StaticModel* boxObject=node->CreateComponent<StaticModel>();
    set_model(boxObject,globals::instance()->cache,"Data/Models/torch");
    boxObject->SetCastShadows(true);
    boxObject->SetOccludee(true);
    boxObject->SetShadowDistance(200);
    boxObject->SetDrawDistance(200);

    auto lightNode=node->CreateChild();
    lightNode->Translate(Vector3(0,2,0));
    Light* light=lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_POINT);
    light->SetRange(50);
    light->SetBrightness(2.0);
    light->SetColor(Color(2.0,1.2,.8,1.0));
    light->SetCastShadows(true);
    light->SetShadowDistance(300);
    light->SetDrawDistance(300);

    auto body_stone=node->CreateComponent<RigidBody>();
    body_stone->SetCollisionLayer(2);
    body_stone->SetMass(50.0);
    body_stone->SetLinearDamping(0.2f);
    body_stone->SetAngularDamping(0.2f);
    body_stone->SetFriction(0.6);
    CollisionShape* shape=node->CreateComponent<CollisionShape>();
    shape->SetBox(Vector3(0.7,1.47,0.7),Vector3(0,1.47/2,0));

    auto n_particle=node->CreateChild();
    n_particle->Translate(Vector3(0,1.6,0));
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
    sound_torch_source->SetFrequency(sound_torch->GetFrequency()*Random(0.7f,1.3f));
}
