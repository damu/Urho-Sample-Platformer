#include "gs_playing.h"

#include "gs_main_menu.h"

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
        boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Textures/3D_pattern_57/pattern_288/material.xml"));
        boxObject->SetCastShadows(true);

        RigidBody* body=boxNode_->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
        CollisionShape* shape=boxNode_->CreateComponent<CollisionShape>();
        shape->SetTriangleMesh(globals::instance()->cache->GetResource<Model>("Models/level_1.mdl"));

        body->GetPhysicsWorld()->SetGravity(Vector3(0,-9.81*2,0));
    }

    {
        node_player=globals::instance()->scene->CreateChild("Player");
        nodes.push_back(node_player);
        node_player->SetPosition(Vector3(4,10,-5));
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
        light->SetShadowBias(BiasParameters(0.0000025f,0.8f));
        light->SetShadowCascade(CascadeParameters(20.0f,50.0f,200.0f,2000.0f,0.01f,0.5f));
        light->SetShadowResolution(10.0);
        light->SetBrightness(1.2);
        light->SetColor(Color(1,.8,.7,1));
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(45);
        lightNode->Roll(30);
        lightNode->Pitch(60);
    }
    {
        Node* lightNode=globals::instance()->camera->GetNode()->CreateChild("Light");
        nodes.push_back(lightNode);
        lightNode->SetPosition(Vector3(0,15,10));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(10);
        light->SetBrightness(2.0);
        light->SetColor(Color(.8,1,.8,1.0));
    }

    {   // "load" flags
        flag_positions.emplace_back(20.1,14,-46.9);
        flag_positions.emplace_back(-10.1,14,-46.9);
        flag_positions.emplace_back(1.1,0.5,-180.4);
        flag_positions.emplace_back(18.2,21.6,-8.1);
        flag_positions.emplace_back(28.7,33.9,82.6);

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
}

void gs_playing::update(StringHash eventType,VariantMap& eventData)
{
    Input* input=GetSubsystem<Input>();
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    framecount_++;
    time_+=timeStep;

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
            str.append(std::to_string(timer_playing.until_now()));
            if(!flag_nodes.size())
                goal_time=timer_playing.until_now();
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
        cam_distance=Clamp(cam_distance,2.0,20.0);

        node_camera->SetPosition(node_player->GetPosition());
        node_camera->SetDirection(Vector3::FORWARD);
        static float yaw=20;
        yaw+=mouseMove.x_*0.1;
        node_camera->Yaw(yaw);
        static float pitch=20;
        pitch+=mouseMove.y_*0.1;
        pitch=Clamp(pitch,-85.0,85.0);
        node_camera->Translate(Vector3(0,1,0));
        node_camera->Pitch(pitch);

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

        if(moveDir.Length()>0.5)
            moveDir.Normalize();

        Vector3 vel=body_player->GetLinearVelocity()*Vector3(1,0,1);
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
                moveDir*=0.2;

            if(input->GetKeyDown(KEY_SPACE)&&jumping==false&&(on_floor||at_wall))
            {
                jumping=1;   // start jumping
                if(at_wall) // to avoid using one wall for walljump chaining
                {
                    auto v=result.normal_*Vector3(1,0,1)+vel*Vector3(1,0,1)*0.2;
                    v.Normalize();
                    body_player->SetLinearVelocity(Vector3(v.x_*10,0,v.z_*10));
                    vel=body_player->GetLinearVelocity()*Vector3(1,0,1);
                }
            }
            else if(!input->GetKeyDown(KEY_SPACE))
                jumping=0;

            static float jump_force_applied=0;
            static const float max_jump_force_applied=500;
            Vector3 moveDir_world=node_player->GetWorldRotation()*moveDir;
            if(jumping==1&&jump_force_applied<max_jump_force_applied)   // jump higher if we are jumping and
            {
                if(jump_force_applied>max_jump_force_applied)
                {
                    // do nothing if max jump force reached
                }
                else if(jump_force_applied+timeStep*2000>max_jump_force_applied)
                {
                    // I want to limit the jump more exactly height by limiting the force pumped into it and applieng the remaining rest here. Doesn't fully work yet.
                    float f=0;//(max_jump_force_applied-jump_force_applied)*timeStep*2000;
                    moveDir+=Vector3::UP*2*f;
                    moveDir_global=result.normal_*1*f;
                    jump_force_applied+=timeStep*2500;
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
                    jump_force_applied+=timeStep*2500;
                }
            }
            if(jumping!=1)
                jump_force_applied=0;
        }

        Quaternion rot=node_player->GetRotation();
        Quaternion quat;
        quat.FromLookRotation(node_camera->GetDirection()*Vector3(1,0,1),Vector3::UP);
        body_player->SetRotation(quat);
        float speed_old=vel.Length();
        vel+=rot*moveDir*timeStep*2500/body_player->GetMass();
        float speed_new=vel.Length();
        if(speed_new>15&&speed_new>speed_old)   // over limit. Don't increase speed further but make direction change possible.
        {
            vel=vel.Normalized()*speed_old;
/*            std::string s;
            s+=std::to_string(speed_old);
            s+=std::to_string(speed_new);
            s+=std::to_string(vel.Length());
            LOGINFO(String(s.data(),s.size()));*/
        }
        body_player->SetLinearVelocity(Vector3(vel.x_,body_player->GetLinearVelocity().y_+(rot*moveDir*timeStep*3000/body_player->GetMass()).y_,vel.z_));
        body_player->ApplyImpulse(moveDir_global*timeStep*2500);

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
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->game_state_.reset(new gs_main_menu);
}
