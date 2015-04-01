#include "gs_main_menu.h"
#include "gs_playing.h"

using namespace Urho3D;

gs_main_menu::gs_main_menu() : game_state()
{
    boxNode_=globals::instance()->scene->CreateChild("Box");
    nodes.push_back(boxNode_);
    boxNode_->SetPosition(Vector3(0,0,5));
    StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
    boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/Stone.mdl"));
    boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/Stone.xml"));

    for(int x=-30;x<30;x+=3)
        for(int y=-30;y<30;y+=3)
        {
            Node* boxNode_=globals::instance()->scene->CreateChild("Box");
            nodes.push_back(boxNode_);
            boxNode_->SetPosition(Vector3(x,-3,y));
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
    }
    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        lightNode->SetPosition(Vector3(5,-3,5));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(50);
        light->SetBrightness(1.2);
        light->SetColor(Color(.5,.8,1,1));
    }
    {
        Node* lightNode=globals::instance()->camera->GetNode()->CreateChild("Light");
        nodes.push_back(lightNode);
        lightNode->SetPosition(Vector3(0,0,10));
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(10);
        light->SetBrightness(2.0);
        light->SetColor(Color(.8,1,.8,1.0));
    }


    Window* window_=new Window(globals::instance()->context);
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
        Button* button = new Button(globals::instance()->context);
        button->SetName("Button");
        button->SetMinHeight(100);
        button->SetStyleAuto();
        {
            Text* t = new Text(globals::instance()->context);
            t->SetHorizontalAlignment(HA_CENTER);
            t->SetVerticalAlignment(VA_CENTER);
            t->SetName("Text");
            t->SetText("Quit");
            //button->AddChild(t);
            t->SetStyle("Text");
            t->SetMinHeight(100);
            button->AddChild(t);
        }
        window_->AddChild(button);
        SubscribeToEvent(button,E_RELEASED,HANDLER(gs_main_menu,HandlePlayPressed));
    }
    {
        Button* button = new Button(globals::instance()->context);
        button->SetName("Button");
        button->SetMinHeight(100);
        button->SetStyleAuto();
        {
            Text* t = new Text(globals::instance()->context);
            t->SetHorizontalAlignment(HA_CENTER);
            t->SetVerticalAlignment(VA_CENTER);
            t->SetName("Text");
            t->SetText("Quit");
            //button->AddChild(t);
            t->SetStyle("Text");
            t->SetMinHeight(100);
            button->AddChild(t);
        }
        window_->AddChild(button);
        SubscribeToEvent(button,E_RELEASED,HANDLER(gs_main_menu,HandleClosePressed));
    }


    GetSubsystem<Input>()->SetMouseVisible(true);
    GetSubsystem<Input>()->SetMouseGrabbed(false);

    SubscribeToEvent(E_UPDATE,HANDLER(gs_main_menu,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_main_menu,HandleKeyDown));
    SubscribeToEvent(buttonClose,E_RELEASED,HANDLER(gs_main_menu,HandleClosePressed));
}

gs_main_menu::~gs_main_menu()
{
}

void gs_main_menu::update(StringHash eventType,VariantMap& eventData)
{
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();

    boxNode_->Rotate(Quaternion(8*timeStep,16*timeStep,0));
}

void gs_main_menu::HandlePlayPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
{
    globals::instance()->game_state_.reset(new gs_playing);
}

void gs_main_menu::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->engine->Exit();
}
