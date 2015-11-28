#include "gs_main_menu.h"
#include "gs_playing.h"

using namespace Urho3D;
using namespace std;

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

        ParticleEmitter* emitter=node_rotating_flag->CreateComponent<ParticleEmitter>();
        emitter->SetEffect(globals::instance()->cache->GetResource<ParticleEffect>("Particle/flag.xml"));

        globals::instance()->physical_world=node_rotating_flag->CreateComponent<RigidBody>()->GetPhysicsWorld();    // there may be better ways to get the physical world object
    }

    // skybox
    {
        Node* skyNode=globals::instance()->scene->CreateChild("Sky");
        nodes.push_back(skyNode);
        skyNode->SetScale(1500.0f);
        Skybox* skybox=skyNode->CreateComponent<Skybox>();
        skybox->SetModel(globals::instance()->cache->GetResource<Model>("Models/Box.mdl"));
        skybox->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/Skybox.xml"));
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

    // grid of 400 cubes, known from the basic sample application at the Urho Wiki
    for(int x=-30;x<30;x+=3)
        for(int y=-30;y<30;y+=3)
        {
            Node* boxNode_=globals::instance()->scene->CreateChild("Box");
            nodes.push_back(boxNode_);
            boxNode_->SetPosition(Vector3(x,-1.5,y));
            boxNode_->SetScale(Vector3(2,2,2));
            StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/Box.mdl"));
            boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/mossy_stone.xml"));
            boxObject->SetCastShadows(true);
        }

    // sun
    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00000025f,0.1f));
        light->SetShadowCascade(CascadeParameters(20.0f,60.0f,180.0f,560.0f,100.0f,100.0f));
        light->SetShadowResolution(1.0);
        light->SetBrightness(1.2);
        light->SetColor(Color(1.5,1.2,1,1));
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(-150);   // horizontal
        lightNode->Pitch(60);   // vertical
        lightNode->Translate(Vector3(0,0,-20000));

        BillboardSet* billboardObject=lightNode->CreateComponent<BillboardSet>();
        billboardObject->SetNumBillboards(1);
        billboardObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/sun.xml"));
        billboardObject->SetSorted(true);
        Billboard* bb=billboardObject->GetBillboard(0);
        bb->size_=Vector2(10000,10000);
        bb->rotation_=Random()*360.0f;
        bb->enabled_=true;
        billboardObject->Commit();
    }

    window_menu=new Window(globals::instance()->context);
    gui_elements.push_back(window_menu);
    globals::instance()->ui_root->AddChild(window_menu);

    window_menu->SetSize(300,400);
    window_menu->SetLayout(LM_FREE,0,IntRect(10,10,10,10));
    window_menu->SetAlignment(HA_CENTER,VA_CENTER);
    window_menu->SetName("Window");
    window_menu->SetColor(Color(.0,.15,.3,.5));
    window_menu->SetStyleAuto();

    {
        lv_levels=new ListView(globals::instance()->context);
        lv_levels->SetPosition(10,10);
        lv_levels->SetFixedSize(280,330);
        lv_levels->SetStyleAuto();
        lv_levels->SetOpacity(0.75);
        window_menu->AddChild(lv_levels);

        FileSystem fs(globals::instance()->context);
        Vector<String> result;
        fs.ScanDir(result,"Data/maps/","*",SCAN_FILES,true);

        for(int i=0;i<result.Size();i++)
        {
            String s=result[i].Substring(0,result[i].FindLast('.'));
            Text* t=new Text(globals::instance()->context);
            t->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
            t->SetText("   "+s);
            t->SetStyle("Text");
            t->SetVar("filename",s);
            t->SetMinHeight(30);
            lv_levels->AddItem(t);
            if(i==0)
                lv_levels->SetSelection(0);
        }
    }

    {
        Button* button=new Button(globals::instance()->context);
        button->SetPosition(10,350);
        button->SetFixedSize(135,40);
        button->SetName("Button");
        button->SetStyleAuto();
        button->SetOpacity(0.75);
        {
            Text* t=new Text(globals::instance()->context);
            t->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
            t->SetHorizontalAlignment(HA_CENTER);
            t->SetVerticalAlignment(VA_CENTER);
            t->SetName("Text");
            t->SetText("Play");
            button->AddChild(t);
        }
        window_menu->AddChild(button);
        SubscribeToEvent(button,E_RELEASED,URHO3D_HANDLER(gs_main_menu,HandlePlayPressed));
    }
    {
        Button* button = new Button(globals::instance()->context);
        button->SetPosition(155,350);
        button->SetFixedSize(135,40);
        button->SetName("Button");
        button->SetStyleAuto();
        button->SetOpacity(0.75);
        {
            Text* t=new Text(globals::instance()->context);
            t->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
            t->SetHorizontalAlignment(HA_CENTER);
            t->SetVerticalAlignment(VA_CENTER);
            t->SetName("Text");
            t->SetText("Quit");
            button->AddChild(t);
        }
        window_menu->AddChild(button);
        SubscribeToEvent(button,E_RELEASED,URHO3D_HANDLER(gs_main_menu,HandleClosePressed));
    }

    GetSubsystem<Input>()->SetMouseVisible(true);
    GetSubsystem<Input>()->SetMouseGrabbed(false);

    SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(gs_main_menu,update));
    SubscribeToEvent(E_KEYDOWN,URHO3D_HANDLER(gs_main_menu,HandleKeyDown));
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

    // The selected item in the list view is not highlighted at all for some reason, so I'm writing a "->" in front of the selected one.
    // Tried doing this with an event (like OnChange or something) but couldn't get that to work.
    for(int i=0;i<lv_levels->GetNumItems();i++)
    {
        Text* t=(Text*)lv_levels->GetItem(i);
        if(lv_levels->IsSelected(i))
            t->SetText("-> "+t->GetVar("filename").GetString()+" ("+std::to_string(highscores.get((t->GetVar("filename").GetString()+".xml").CString())).c_str()+"s)");
        else
            t->SetText("   "+t->GetVar("filename").GetString()+" ("+std::to_string(highscores.get((t->GetVar("filename").GetString()+".xml").CString())).c_str()+"s)");
    }
}

void gs_main_menu::HandlePlayPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData)
{
    Text* t=(Text*)lv_levels->GetSelectedItem();
    if(!t)
        return;

    globals::instance()->game_states[0].reset(new gs_playing(("maps/"+t->GetVar("filename").GetString()+".xml").CString()));
}

void gs_main_menu::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->engine->Exit();
    else if(key==KEY_G)
        window_menu->SetVisible(!window_menu->IsVisible());
}
