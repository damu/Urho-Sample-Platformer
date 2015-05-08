#include "gs_main_menu.h"
#include "gs_playing.h"

using namespace Urho3D;

gs_main_menu::gs_main_menu() : game_state()
{
    Node* node_camera=globals::instance()->camera->GetNode();
    node_camera->SetPosition(Vector3(0,0,0));
    node_camera->SetDirection(Vector3::FORWARD);

    // a rotating flag
    {
        node_rotating_flag=globals::instance()->scene->CreateChild("Flag");
        nodes.push_back(node_rotating_flag);
        node_rotating_flag->SetPosition(Vector3(0,-0.5,6));
        StaticModel* boxObject=node_rotating_flag->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/flag.mdl"));
        boxObject->SetMaterial(0,globals::instance()->cache->GetResource<Material>("Materials/flag_pole.xml"));
        boxObject->SetMaterial(1,globals::instance()->cache->GetResource<Material>("Materials/flag_cloth.xml"));
        boxObject->SetCastShadows(true);
    }

    // a torch with a light, sound and particle effects
    {
        Node* node=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(node);
        Vector3 pos(Vector3(3,-0.5,6));
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
        light->SetBrightness(1.2);
        light->SetColor(Color(1.0,.6,.4,1.0));
        light->SetCastShadows(true);
        light->SetShadowDistance(200);
        light->SetDrawDistance(200);

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
    }

    // grid of 400 cubes, know from the
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
            t->SetText("Play");
            //button->AddChild(t);
            t->SetStyle("Text");
            t->SetMinHeight(VA_CENTER);
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
        SubscribeToEvent(button,E_RELEASED,HANDLER(gs_main_menu,HandleClosePressed));

    }

    GetSubsystem<Input>()->SetMouseVisible(true);
    GetSubsystem<Input>()->SetMouseGrabbed(false);

    SubscribeToEvent(E_UPDATE,HANDLER(gs_main_menu,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_main_menu,HandleKeyDown));
    SubscribeToEvent(buttonClose,E_RELEASED,HANDLER(gs_main_menu,HandleClosePressed));
}

void gs_main_menu::update(StringHash eventType,VariantMap& eventData)
{
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();

    node_rotating_flag->Rotate(Quaternion(0,64*timeStep,0));

    // Movement speed as world units per second
    float MOVE_SPEED=10.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY=0.1f;

    // camera movement
    Input* input=GetSubsystem<Input>();
    Node* cameraNode_=globals::instance()->camera->GetNode();
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

void gs_main_menu::HandlePlayPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
{
    globals::instance()->game_states[0].reset(new gs_playing);
}

void gs_main_menu::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->engine->Exit();
}
