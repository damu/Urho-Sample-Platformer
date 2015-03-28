/**
* Released under the same permissive MIT-license as Urho3D.
* https://raw.githubusercontent.com/urho3d/Urho3D/master/License.txt
*/

#include <string>

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

using namespace Urho3D;

class MyApp : public Application
{
public:
    int framecount_;
    float time_;
    SharedPtr<Text> text_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> boxNode_;
    Node* cameraNode_;

    MyApp(Context * context) : Application(context),framecount_(0),time_(0)
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

        GetSubsystem<Input>()->SetMouseVisible(true);
        GetSubsystem<Input>()->SetMouseGrabbed(false);

        GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
        text_=new Text(context_);
        text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
        text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
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

        scene_=new Scene(context_);
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<DebugRenderer>();

        Node* skyNode=scene_->CreateChild("Sky");
        skyNode->SetScale(500.0f); // The scale actually does not matter
        Skybox* skybox=skyNode->CreateComponent<Skybox>();
        skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

        boxNode_=scene_->CreateChild("Box");
        boxNode_->SetPosition(Vector3(0,0,5));
        StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

        for(int x=-30;x<30;x+=3)
            for(int y=-30;y<30;y+=3)
            {
                Node* boxNode_=scene_->CreateChild("Box");
                boxNode_->SetPosition(Vector3(x,-3,y));
                StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
                boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
                boxObject->SetCastShadows(true);
            }

        cameraNode_=scene_->CreateChild("Camera");
        Camera* camera=cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(2000);

        {
            Node* lightNode=scene_->CreateChild("Light");
            lightNode->SetPosition(Vector3(-5,10,5));
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_POINT);
            light->SetRange(50);
            light->SetBrightness(1.2);
            light->SetColor(Color(1,.5,.8,1));
        }
        {
            Node* lightNode=scene_->CreateChild("Light");
            lightNode->SetPosition(Vector3(5,-3,5));
            Light* light=lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_POINT);
            light->SetRange(50);
            light->SetBrightness(1.2);
            light->SetColor(Color(.5,.8,1,1));
        }
        {
            Light* light=cameraNode_->CreateComponent<Light>();
            light->SetLightType(LIGHT_POINT);
            light->SetRange(10);
            light->SetBrightness(2.0);
            light->SetColor(Color(.8,1,.8,1.0));
        }

        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);

        SubscribeToEvent(E_BEGINFRAME,HANDLER(MyApp,HandleBeginFrame));
        SubscribeToEvent(E_KEYDOWN,HANDLER(MyApp,HandleKeyDown));
        SubscribeToEvent(E_UIMOUSECLICK,HANDLER(MyApp,HandleControlClicked));
        SubscribeToEvent(E_UPDATE,HANDLER(MyApp,HandleUpdate));
        SubscribeToEvent(E_POSTUPDATE,HANDLER(MyApp,HandlePostUpdate));
        SubscribeToEvent(E_RENDERUPDATE,HANDLER(MyApp,HandleRenderUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE,HANDLER(MyApp,HandlePostRenderUpdate));
        SubscribeToEvent(E_ENDFRAME,HANDLER(MyApp,HandleEndFrame));
    }
    virtual void Stop()
    {
    }

    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();
        if(key==KEY_ESC)
            engine_->Exit();

        if(key==KEY_TAB)
        {
            GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
            GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
        }
    }

    void HandleControlClicked(StringHash eventType,VariantMap& eventData)
    {
        // Query the clicked UI element.
        UIElement* clicked=static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());
        if(clicked)
            if(clicked->GetName()=="Button Quit")   // check if the quit button was clicked
                engine_->Exit();
    }

    void HandleUpdate(StringHash eventType,VariantMap& eventData)
    {
        float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
        framecount_++;
        time_+=timeStep;
        // Movement speed as world units per second
        float MOVE_SPEED=10.0f;
        // Mouse sensitivity as degrees per pixel
        const float MOUSE_SENSITIVITY=0.1f;

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

        // Rotate the box thingy.
        // A much nicer way of doing this would be with a LogicComponent.
        // With LogicComponents it is easy to control things like movement
        // and animation from some IDE, console or just in game.
        // Alas, it is out of the scope for our simple example.
        boxNode_->Rotate(Quaternion(8*timeStep,16*timeStep,0));

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
            // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
            IntVector2 mouseMove=input->GetMouseMove();
            // avoid the weird extrem values before moving the mouse
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

    void HandleBeginFrame(StringHash eventType,VariantMap& eventData)
    {
    }
    void HandlePostUpdate(StringHash eventType,VariantMap& eventData)
    {
    }
    void HandleRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
    }
    void HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
    }
    void HandleEndFrame(StringHash eventType,VariantMap& eventData)
    {
    }
};

DEFINE_APPLICATION_MAIN(MyApp)
