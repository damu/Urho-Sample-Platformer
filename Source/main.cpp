/**
* Released under the same permissive MIT-license as Urho3D.
* https://raw.githubusercontent.com/urho3d/Urho3D/master/License.txt
*/

#include <string>
#include <memory>

#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/RenderPath.h>
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
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>

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
        //engineParameters_["Multisample"]=16;
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
        SoundListener* listener=cameraNode_->CreateComponent<SoundListener>();
        GetSubsystem<Audio>()->SetListener(listener);

        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);
        renderer->SetShadowMapSize(1024);
        renderer->SetHDRRendering(true);

        RenderPath* effectRenderPath=viewport->GetRenderPath();
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/AutoExposure.xml"));
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/BloomHDR.xml"));
        effectRenderPath->Append(cache->GetResource<XMLFile>("PostProcess/FXAA2.xml"));
        //effectRenderPath.SetEnabled("GammaCorrection",false);

        Node* zoneNode=scene_->CreateChild("Zone");
        Zone* zone=zoneNode->CreateComponent<Zone>();
        zone->SetBoundingBox(BoundingBox(-20000.0f,20000.0f));
        zone->SetFogStart(100.0f);
        zone->SetFogEnd(5000.0f);
        zone->SetAmbientColor(Color(0.1,0.1,0.1));

        SubscribeToEvent(E_KEYDOWN,HANDLER(MyApp,HandleKeyDown));
        SubscribeToEvent(E_UPDATE,HANDLER(MyApp,HandleUpdate));

        // fill our game state shared variables
        globals::instance()->cache=cache;
        globals::instance()->camera=camera;
        globals::instance()->scene=scene_;
        globals::instance()->context=context_;
        globals::instance()->ui_root=GetSubsystem<UI>()->GetRoot();
        globals::instance()->engine=engine_;
        globals::instance()->game_states.emplace_back(new gs_main_menu);
    }
    virtual void Stop()
    {
        globals::instance()->game_states.resize(0);
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

    }
};

DEFINE_APPLICATION_MAIN(MyApp)
