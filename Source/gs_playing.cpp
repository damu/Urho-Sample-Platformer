#include "gs_playing.h"

#include "gs_main_menu.h"

using namespace Urho3D;

gs_playing::gs_playing() : game_state()
{
    text_=new Text(context_);
    text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
    text_->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
    text_->SetColor(Color(.3,0,.3));
    text_->SetHorizontalAlignment(HA_CENTER);
    text_->SetVerticalAlignment(VA_TOP);
    GetSubsystem<UI>()->GetRoot()->AddChild(text_);
    Button* button=new Button(context_);
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
        auto boxNode_=globals::instance()->scene->CreateChild("Box");
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
    }

    {
        node_player=globals::instance()->scene->CreateChild("Player");
        nodes.push_back(node_player);
        node_player->SetPosition(Vector3(4,10,-5));
        node_player_model=globals::instance()->scene->CreateChild();
        nodes.push_back(node_player_model);
        StaticModel* boxObject=node_player_model->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/Jack.mdl"));
        boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/Jack.xml"));
        boxObject->SetCastShadows(true);

        body_player=node_player->CreateComponent<RigidBody>();
        body_player->SetCollisionLayer(1);
        body_player->SetMass(80.0);
        body_player->SetLinearDamping(0.01f);
        body_player->SetAngularDamping(0.98f);
        body_player->SetAngularFactor(Vector3(0,1,0));
        body_player->SetFriction(0.8);
        CollisionShape* shape=node_player->CreateComponent<CollisionShape>();
        shape->SetCapsule(1,2,Vector3(0,1,0));
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
}

void gs_playing::update(StringHash eventType,VariantMap& eventData)
{
    Input* input=GetSubsystem<Input>();
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    framecount_++;
    time_+=timeStep;
    if(time_ >=1)
    {
        std::string str;
        str.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
        str.append(std::to_string(framecount_));
        str.append(" frames in ");
        str.append(std::to_string(time_));
        str.append(" seconds = ");
        str.append(std::to_string((float)framecount_ / time_));
        str.append(" fps");
        String s(str.c_str(),str.size());
        text_->SetText(s);
        framecount_=0;
        time_=0;
    }

    IntVector2 mouseMove(0,0);
    if(!input->IsMouseVisible())
        mouseMove=input->GetMouseMove();

    Node* node_camera=globals::instance()->camera->GetNode();
    {
        node_camera->SetPosition(node_player->GetPosition());
        node_camera->SetDirection(Vector3::FORWARD);
        static float yaw=20;
        yaw+=mouseMove.x_*0.1;
        node_camera->Yaw(yaw);
        static float pitch=20;
        pitch+=mouseMove.y_*0.1;
        pitch=Clamp(pitch,-90.0,90.0);
        node_camera->Pitch(pitch);
        node_camera->Translate(Vector3(0,2,-8));
    }
    {
        Vector3 moveDir=Vector3::ZERO;
        Vector3 moveDir_global=Vector3::ZERO;
        if(input->GetKeyDown('D'))
            //torqueVec+=Vector3(0,1,0)*0.5;
            moveDir+=Vector3::RIGHT*1;
        if(input->GetKeyDown('A'))
            //torqueVec+=Vector3(0,-1,0)*0.5;
            moveDir-=Vector3::RIGHT*1;
        if(input->GetKeyDown('W'))
            moveDir+=Vector3::FORWARD*1;
        if(input->GetKeyDown('S'))
            moveDir-=Vector3::FORWARD*1;

        if(moveDir.Length()>0.5)
            moveDir.Normalize();

        {
            static bool on_floor;
            static bool at_wall;
            static int jumping=0; // 0 = not jumping, 1 = jumping, 2 =
            on_floor=false;
            at_wall=false;

            float height=0;
            PhysicsRaycastResult result;
            Ray ray(node_player->GetPosition()+Vector3(0,1.0,0),Vector3::DOWN);
            body_player->GetPhysicsWorld()->SphereCast(result,ray,0.8,3,2);
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
            }
            else if(!input->GetKeyDown(KEY_SPACE))
            {
                jumping=0;
            }

            static float jump_force_applied=0;
            static const float max_jump_force_applied=400;
            if(jumping==1&&jump_force_applied<max_jump_force_applied)   // jump higher if we are jumping and
            {
                if(jump_force_applied>max_jump_force_applied)
                {
                    // do nothing if max jump force reached
                }
                else if(jump_force_applied+timeStep*2000>max_jump_force_applied)
                {
                    // I want to limit the jump height by limiting the force pumped into it. Doesn't fully work yet.
                    float f=0;//(max_jump_force_applied-jump_force_applied)*timeStep*2000;
                    moveDir+=Vector3::UP*1.5*f;
                    moveDir_global=result.normal_*1.5*f;
                    jump_force_applied+=timeStep*2000;
                }
                else
                {
                    jump_force_applied+=timeStep*2000;
                    moveDir+=Vector3::UP*1.5;
                    moveDir_global=result.normal_*1.5;
                }
            }
            if(jumping!=1)
                jump_force_applied=0;
        }

        Quaternion rot=node_player->GetRotation();
        Quaternion quat;
        quat.FromLookRotation(node_camera->GetDirection()*Vector3(1,0,1),Vector3::UP);
        body_player->SetRotation(quat);
        body_player->ApplyImpulse(moveDir_global*timeStep*2000);
        Vector3 vel=body_player->GetLinearVelocity()*Vector3(1,0,1);
        float speed_old=vel.Length();
        vel+=rot*moveDir*timeStep*2000/body_player->GetMass();
        float speed_new=vel.Length();
        if(speed_new>15&&speed_new>speed_old)   // over limit. Don't increase speed further but make direction change possible.
        {
            vel=vel.Normalized()*speed_old;
            std::string s;
            s+=std::to_string(speed_old);
            s+=std::to_string(speed_new);
            s+=std::to_string(vel.Length());
            LOGINFO(String(s.data(),s.size()));
        }
        body_player->SetLinearVelocity(Vector3(vel.x_,body_player->GetLinearVelocity().y_+(rot*moveDir*timeStep*2000/body_player->GetMass()).y_,vel.z_));

        auto vec_rot=body_player->GetLinearVelocity()*Vector3(1,0,1);
        float s=vec_rot.Length();
        vec_rot.Normalize();
        float yaw=asin(vec_rot.x_)*180/M_PI;
        if(vec_rot.z_<0)
            yaw=-yaw-180;
        node_player_model->SetPosition(node_player->GetPosition());
        if(s>0.5)
        {
            node_player_model->SetDirection(Vector3::FORWARD);
            node_player_model->Yaw(yaw);
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
