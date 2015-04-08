/**
* Released under the same permissive MIT-license as Urho3D.
* https://raw.githubusercontent.com/urho3d/Urho3D/master/License.txt
*/

#include <string>
#include <memory>

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/Graphics/Zone.h>

#include "gs_main_menu.h"
#include "gs_playing.h"

using namespace Urho3D;

class MyApp : public Application
{
public:
    SharedPtr<Scene> scene_;
    Node* cameraNode_;

    MyApp(Context * context) : Application(context)
    {
    }

    virtual void Setup()
    {
        engineParameters_["FullScreen"]=false;
        engineParameters_["WindowWidth"]=1280;
        engineParameters_["WindowHeight"]=720;
        engineParameters_["WindowResizable"]=true;
    }

    virtual void Start()
    {
        ResourceCache* cache=GetSubsystem<ResourceCache>();
        GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));

        scene_=new Scene(context_);
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<DebugRenderer>();

        // until we get a better sky system
        Node* skyNode=scene_->CreateChild("Sky");
        skyNode->SetScale(500.0f);
        Skybox* skybox=skyNode->CreateComponent<Skybox>();
        skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

        cameraNode_=scene_->CreateChild("Camera");
        Camera* camera=cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(10000);

        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);

        Node* zoneNode=scene_->CreateChild("Zone");
        Zone* zone=zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-20000.0f,20000.0f));
        zone->SetFogStart(100.0f);
        zone->SetFogEnd(5000.0f);
        zone->SetAmbientColor(Color(0.3,0.3,0.3));

        SubscribeToEvent(E_KEYDOWN,HANDLER(MyApp,HandleKeyDown));
        SubscribeToEvent(E_UPDATE,HANDLER(MyApp,HandleUpdate));

        globals::instance()->cache=cache;
        globals::instance()->camera=camera;
        globals::instance()->scene=scene_;
        globals::instance()->context=context_;
        globals::instance()->ui_root=GetSubsystem<UI>()->GetRoot();
        globals::instance()->engine=engine_;
        globals::instance()->game_state_.reset(new gs_main_menu);
    }
    virtual void Stop()
    {
        globals::instance()->game_state_.reset();
    }

    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();

        if(key==KEY_TAB)
        {
            GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
            GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
        }
    }

    void HandleUpdate(StringHash eventType,VariantMap& eventData)
    {
        float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
        // Movement speed as world units per second
        float MOVE_SPEED=10.0f;
        // Mouse sensitivity as degrees per pixel
        const float MOUSE_SENSITIVITY=0.1f;

        Input* input=GetSubsystem<Input>();
        if(input->GetQualifierDown(1))  // 1 is shift, 2 is ctrl, 4 is alt
            MOVE_SPEED*=10;
        if(input->GetKeyDown('W'))
            cameraNode_->Translate(Vector3(0,0, 1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('S'))
            cameraNode_->Translate(Vector3(0,0,-1)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('A'))
            cameraNode_->Translate(Vector3(-1,0,0)*MOVE_SPEED*timeStep);
        if(input->GetKeyDown('D'))
            cameraNode_->Translate(Vector3( 1,0,0)*MOVE_SPEED*timeStep);

        if(!GetSubsystem<Input>()->IsMouseVisible())
        {
            IntVector2 mouseMove=input->GetMouseMove();
            if(mouseMove.x_>-2000000000&&mouseMove.y_>-2000000000)
            {
                static float yaw_=0;
                static float pitch_=0;
                yaw_+=MOUSE_SENSITIVITY*mouseMove.x_;
                pitch_+=MOUSE_SENSITIVITY*mouseMove.y_;
                pitch_=Clamp(pitch_,-90.0f,90.0f);
                // Reset rotation and set yaw and pitch again
                cameraNode_->SetDirection(Vector3::FORWARD);
                cameraNode_->Yaw(yaw_);
                cameraNode_->Pitch(pitch_);
            }
        }
    }
};

DEFINE_APPLICATION_MAIN(MyApp)
