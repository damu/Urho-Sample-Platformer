#include "gs_level_end.h"
#include "gs_main_menu.h"

using namespace Urho3D;

gs_level_end::gs_level_end() : game_state()
{
    Node* node_camera=globals::instance()->camera->GetNode();
    node_camera->SetPosition(Vector3(0,0,0));
    node_camera->SetDirection(Vector3::FORWARD);

    boxNode_=globals::instance()->scene->CreateChild("Flag");
    nodes.push_back(boxNode_);
    boxNode_->SetPosition(Vector3(0,-0.5,6));
    StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
    boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/flag.mdl"));
    boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/flag_pole.xml"));
    boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/flag_cloth.xml"));
    boxObject->SetCastShadows(true);

    for(int x=-30;x<30;x+=3)
        for(int y=-30;y<30;y+=3)
        {
            Node* boxNode_=globals::instance()->scene->CreateChild("Box");
            nodes.push_back(boxNode_);
            boxNode_->SetPosition(Vector3(x,-1,y));
            StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/Box.mdl"));
            boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/Stone.xml"));
            boxObject->SetCastShadows(true);
        }
    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        lightNode->SetPosition(Vector3(-5,10,5));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(1,.5,.8,1));
        light->SetCastShadows(true);
    }
    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        lightNode->SetPosition(Vector3(5,2,5));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(.5,.8,1,1));
        light->SetCastShadows(true);
    }
    {
        Node* lightNode=globals::instance()->camera->GetNode()->CreateChild("Light");
        nodes.push_back(lightNode);
        lightNode->SetPosition(Vector3(0,2,10));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(10);
        light->SetBrightness(2.0);
        light->SetColor(Color(.8,1,.8,1.0));
        light->SetCastShadows(true);
    }


    Window* window_=new Window(globals::instance()->context);
    gui_elements.push_back(window_);
    globals::instance()->ui_root->AddChild(window_);

    window_->SetMinSize(384,192);
    window_->SetLayout(LM_VERTICAL,6,IntRect(6,6,6,6));
    window_->SetAlignment(HA_CENTER,VA_CENTER);
    window_->SetName("Window");

    UIElement* titleBar=new UIElement(globals::instance()->context);
    titleBar->SetMinSize(0,24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    Text* windowTitle=new Text(globals::instance()->context);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("Hello GUI!");
    windowTitle->SetStyleAuto();
    titleBar->AddChild(windowTitle);

    Button* buttonClose=new Button(globals::instance()->context);
    buttonClose->SetName("CloseButton");
    buttonClose->SetStyle("CloseButton");
    titleBar->AddChild(buttonClose);

    window_->AddChild(titleBar);
    window_->SetStyleAuto();

    {
        Button* button=new Button(globals::instance()->context);
        button->SetName("Button");
        button->SetMinHeight(100);
        button->SetStyleAuto();
        {
            Text* t=new Text(globals::instance()->context);
            t->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
            t->SetHorizontalAlignment(HA_CENTER);
            t->SetVerticalAlignment(VA_CENTER);
            t->SetName("Text");
            t->SetText("Play");
            //button->AddChild(t);
            t->SetStyle("Text");
            t->SetMinHeight(VA_CENTER);
            button->AddChild(t);

        }
        window_->AddChild(button);
        SubscribeToEvent(button,E_RELEASED,HANDLER(gs_level_end,HandlePlayPressed));
    }
    {
        Button* button = new Button(globals::instance()->context);
        button->SetName("Button");
        button->SetMinHeight(100);
        button->SetStyleAuto();
        {
            Text* t = new Text(globals::instance()->context);
            t->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
            t->SetHorizontalAlignment(HA_CENTER);
            t->SetVerticalAlignment(VA_CENTER);
            t->SetName("Text");
            t->SetText("Quit");
            //button->AddChild(t);
            t->SetStyle("Text");
            t->SetMinHeight(VA_CENTER);
            button->AddChild(t);
        }
        window_->AddChild(button);
        SubscribeToEvent(button,E_RELEASED,HANDLER(gs_level_end,HandleClosePressed));

    }


    GetSubsystem<Input>()->SetMouseVisible(true);
    GetSubsystem<Input>()->SetMouseGrabbed(false);

    SubscribeToEvent(E_UPDATE,HANDLER(gs_level_end,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_level_end,HandleKeyDown));
    SubscribeToEvent(buttonClose,E_RELEASED,HANDLER(gs_level_end,HandleClosePressed));
}

void gs_level_end::update(StringHash eventType,VariantMap& eventData)
{
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();

    boxNode_->Rotate(Quaternion(0,64*timeStep,0));
}

void gs_level_end::HandlePlayPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
{
    globals::instance()->game_states[0].reset(new gs_main_menu);
}

void gs_level_end::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->engine->Exit();
}
