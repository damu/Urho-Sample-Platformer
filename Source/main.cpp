/**
* A simple Urho3D example in one (big) file.
* Copyright 2014 Peter Gebauer, 2015 gawag.
* Released under the same permissive MIT-license as Urho3D.
* https://raw.githubusercontent.com/urho3d/Urho3D/master/License.txt
*
* Why?
* Because A first "simple" example tutorial shouldn't require additional
* frameworks or special toolchains. This file along with Urho3D and a C++
* compiler should do it. (you might have to change the prefix path in
* MyApp::Setup)
* Many (like me) want to learn themselves and just get an overview
* without the overhead of understanding how the example is built.
* I hope this file covers the basics and is of use to you.
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
/**
* Using the convenient Application API we don't have
* to worry about initializing the engine or writing a main.
* You can probably mess around with initializing the engine
* and running a main manually, but this is convenient and portable.
*/
class MyApp : public Application
{
public:
    int framecount_;
    float time_;
    SharedPtr<Text> text_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> boxNode_;
    Node* cameraNode_;

    /**
    * This happens before the engine has been initialized
    * so it's usually minimal code setting defaults for
    * whatever instance variables you have.
    * You can also do this in the Setup method.
    */
    MyApp(Context * context) : Application(context),framecount_(0),time_(0)
    {
    }

    /**
    * This method is called _before_ the engine has been initialized.
    * Thusly, we can setup the engine parameters before anything else
    * of engine importance happens (such as windows, search paths,
    * resolution and other things that might be user configurable).
    */
    virtual void Setup()
    {
        // These parameters should be self-explanatory.
        // See http://urho3d.github.io/documentation/1.32/_main_loop.html
        // for a more complete list.
        engineParameters_["FullScreen"]=false;
        engineParameters_["WindowWidth"]=1280;
        engineParameters_["WindowHeight"]=720;
        engineParameters_["WindowResizable"]=true;
        // Override the resource prefix path to use. "If not specified then the
        // default prefix path is set to URHO3D_PREFIX_PATH environment
        // variable (if defined) or executable path."
        // By default mine was in /usr/local/share, change as needed.
        // Remember to use a TRAILING SLASH to your path! (for unknown reason)
        //engineParameters_["ResourcePrefixPath"] = "/usr/local/share/Urho3D/Bin/";
    }

    /**
    * This method is called _after_ the engine has been initialized.
    * This is where you set up your actual content, such as scenes,
    * models, controls and what not. Basically, anything that needs
    * the engine initialized and ready goes in here.
    */
    virtual void Start()
    {
        // We will be needing to load resources.
        // All the resources used in this example comes with Urho3D.
        // If the engine can't find them, check the ResourcePrefixPath.
        ResourceCache* cache=GetSubsystem<ResourceCache>();

        // Seems like the mouse must be in cursor mode before creating the UI or it won't work.
        GetSubsystem<Input>()->SetMouseVisible(true);
        GetSubsystem<Input>()->SetMouseGrabbed(false);

        // Let's use the default style that comes with Urho3D.
        GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
        // Let's create some text to display.
        text_=new Text(context_);
        // Text will be updated later in the E_UPDATE handler. Keep readin'.
        text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
        // If the engine cannot find the font, it comes with Urho3D.
        // Set the environment variables URHO3D_HOME, URHO3D_PREFIX_PATH or
        // change the engine parameter "ResourcePrefixPath" in the Setup method.
        text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
        text_->SetColor(Color(.3,0,.3));
        text_->SetHorizontalAlignment(HA_CENTER);
        text_->SetVerticalAlignment(VA_TOP);
        GetSubsystem<UI>()->GetRoot()->AddChild(text_);
        // Add a button, just as an interactive UI sample.
        Button* button=new Button(context_);
        // Note, must be part of the UI system before SetSize calls!
        GetSubsystem<UI>()->GetRoot()->AddChild(button);
        button->SetName("Button Quit");
        button->SetStyle("Button");
        //button->SetText("quit");
        button->SetSize(32,32);
        button->SetPosition(16,16);

        // Now we can change the mouse mode.
        GetSubsystem<Input>()->SetMouseVisible(false);
        GetSubsystem<Input>()->SetMouseGrabbed(true);

        // Let's setup a scene to render.
        scene_=new Scene(context_);
        // Let the scene have an Octree component!
        scene_->CreateComponent<Octree>();
        // Let's add an additional scene component for fun.
        scene_->CreateComponent<DebugRenderer>();

        // Let's put some sky in there.
        // Again, if the engine can't find these resources you need to check
        // the "ResourcePrefixPath". These files come with Urho3D.
        Node* skyNode=scene_->CreateChild("Sky");
        skyNode->SetScale(500.0f); // The scale actually does not matter
        Skybox* skybox=skyNode->CreateComponent<Skybox>();
        skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

        // Let's put a box in there.
        boxNode_=scene_->CreateChild("Box");
        boxNode_->SetPosition(Vector3(0,0,5));
        StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
        boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
        boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

        // Create a plane out of 900 boxes.
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

        // We need a camera from which the viewport can render.
        cameraNode_=scene_->CreateChild("Camera");
        Camera* camera=cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(2000);

        // Create two lights
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
        // add one to the camera node as well
        {
            Light* light=cameraNode_->CreateComponent<Light>();
            light->SetLightType(LIGHT_POINT);
            light->SetRange(10);
            light->SetBrightness(2.0);
            light->SetColor(Color(.8,1,.8,1.0));
        }

        // Now we setup the viewport. Ofcourse, you can have more than one!
        Renderer* renderer=GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_,scene_,cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0,viewport);

        // We subscribe to the events we'd like to handle.
        // In this example we will be showing what most of them do,
        // but in reality you would only subscribe to the events
        // you really need to handle.
        // These are sort of subscribed in the order in which the engine
        // would send the events. Read each handler method's comment for
        // details.
        SubscribeToEvent(E_BEGINFRAME,HANDLER(MyApp,HandleBeginFrame));
        SubscribeToEvent(E_KEYDOWN,HANDLER(MyApp,HandleKeyDown));
        SubscribeToEvent(E_UIMOUSECLICK,HANDLER(MyApp,HandleControlClicked));
        SubscribeToEvent(E_UPDATE,HANDLER(MyApp,HandleUpdate));
        SubscribeToEvent(E_POSTUPDATE,HANDLER(MyApp,HandlePostUpdate));
        SubscribeToEvent(E_RENDERUPDATE,HANDLER(MyApp,HandleRenderUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE,HANDLER(MyApp,HandlePostRenderUpdate));
        SubscribeToEvent(E_ENDFRAME,HANDLER(MyApp,HandleEndFrame));
    }

    /**
    * Good place to get rid of any system resources that requires the
    * engine still initialized. You could do the rest in the destructor,
    * but there's no need, this method will get called when the engine stops,
    * for whatever reason (short of a segfault).
    */
    virtual void Stop()
    {
    }

    /**
    * Every frame's life must begin somewhere. Here it is.
    */
    void HandleBeginFrame(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }

    /**
    * Input from keyboard is handled here. I'm assuming that Input, if
    * available, will be handled before E_UPDATE.
    */
    void HandleKeyDown(StringHash eventType,VariantMap& eventData)
    {
        using namespace KeyDown;
        int key=eventData[P_KEY].GetInt();
        // T'is a good default key for quit things.
        if(key==KEY_ESC)
            engine_->Exit();

        if(key==KEY_TAB)
        {
            GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
            GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
        }
    }

    /**
    * You can get these events from when ever the user interacts with the UI.
    */
    void HandleControlClicked(StringHash eventType,VariantMap& eventData)
    {
        // Query the clicked UI element.
        UIElement* clicked=static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());
        if(clicked)
            if(clicked->GetName()=="Button Quit")   // check if the quit button was clicked
                engine_->Exit();
    }
    /**
    * Your non-rendering logic should be handled here.
    * This could be moving objects, checking collisions and reaction, etc.
    */
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
    /**
    * Anything in the non-rendering logic that requires a second pass,
    * it might be well suited to be handled here.
    */
    void HandlePostUpdate(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * If you have any details you want to change before the viewport is
    * rendered, try putting it here.
    * See http://urho3d.github.io/documentation/1.32/_rendering.html
    * for details on how the rendering pipeline is setup.
    */
    void HandleRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
    /**
    * After everything is rendered, there might still be things you wish
    * to add to the rendering. At this point you cannot modify the scene,
    * only post rendering is allowed. Good for adding things like debug
    * artifacts on screen or brush up lighting, etc.
    */
    void HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
    {
        // We could draw some debuggy looking thing for the octree.
        // scene_->GetComponent<Octree>()->DrawDebugGeometry(true);
    }
    /**
    * All good things must come to an end.
    */
    void HandleEndFrame(StringHash eventType,VariantMap& eventData)
    {
        // We really don't have anything useful to do here for this example.
        // Probably shouldn't be subscribing to events we don't care about.
    }
};

/**
* This macro is expaneded to (roughly, depending on OS) this:
*
* > int RunApplication()
* > {
* > Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
* > Urho3D::SharedPtr<className> application(new className(context));
* > return application->Run();
* > }
* >
* > int main(int argc, char** argv)
* > {
* > Urho3D::ParseArguments(argc, argv);
* > return function;
* > }
*/
DEFINE_APPLICATION_MAIN(MyApp)
