#include "gs_playing.h"

#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/ThirdParty/PugiXml/pugixml.hpp>

#include "gs_pause.h"
#include "gs_level_end.h"
#include "gs_death.h"
#include "player.h"

using namespace Urho3D;

std::string gs_playing::last_level_filename;

gs_playing::gs_playing(std::string level_filename) : game_state()
{
    last_level_filename=level_filename;
    // create a transparent window with some text to display things like level time, remaining flags and FPS
    {
        Window* window=new Window(context_);
        gui_elements.push_back(window);
        GetSubsystem<UI>()->GetRoot()->AddChild(window);
        window->SetStyle("Window");
        window->SetSize(700,100);
        window->SetColor(Color(.0,.15,.3,.5));
        window->SetAlignment(HA_CENTER,VA_TOP);

        text_=new Text(context_);
        text_->SetFont(globals::instance()->cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"),20);
        text_->SetColor(Color(.8,.85,.9));
        text_->SetAlignment(HA_CENTER,VA_CENTER);
        window->AddChild(text_);
    }

    GetSubsystem<Input>()->SetMouseVisible(false);
    GetSubsystem<Input>()->SetMouseGrabbed(true);

    SubscribeToEvent(E_UPDATE,HANDLER(gs_playing,update));
    SubscribeToEvent(E_KEYDOWN,HANDLER(gs_playing,HandleKeyDown));

    load_level(level_filename);

    {
        Node* lightNode=globals::instance()->scene->CreateChild("Light");
        nodes.push_back(lightNode);
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00000025f,1.0f));
        light->SetShadowCascade(CascadeParameters(20.0f,60.0f,180.0f,560.0f,100.0f,100.0f));
        light->SetShadowResolution(1.0);
        light->SetBrightness(1.2);
        light->SetColor(Color(1.5,1.2,1,1));
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(-150);     // horizontal
        lightNode->Pitch(60);   // vertical
    }

    // spawn one rock and remove it to cache the collider mesh (to avoid a ~1 second lag when spawning the first rock during the game)
    {
        auto node_stone=globals::instance()->scene->CreateChild("Stone");
        StaticModel* boxObject=node_stone->CreateComponent<StaticModel>();
        boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
        boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/rock.xml"));
        boxObject->SetCastShadows(true);
        float s=1.0+Random(3.0f);
        node_stone->SetScale(s);

        PhysicsRaycastResult result;
        Vector3 pos(-120-Random(100.0f),100,-70-Random(100.0f));
        Ray ray(pos,Vector3(0,-1,0));
        globals::instance()->physical_world->SphereCast(result,ray,2,100);
        if(result.distance_<=1000)
            pos=result.position_+Vector3(0,5,0);

        auto body_stone=node_stone->CreateComponent<RigidBody>();
        body_stone->SetPosition(pos);
        body_stone->SetCollisionLayer(2);
        body_stone->SetMass(50.0*s*s);
        body_stone->SetLinearDamping(0.2f);
        body_stone->SetAngularDamping(0.2f);
        //body_stone->SetAngularFactor(Vector3(0,1,0));
        body_stone->SetFriction(0.6);
        CollisionShape* shape=node_stone->CreateComponent<CollisionShape>();
        //shape->SetCapsule(1,1.2);
        shape->SetConvexHull(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
        node_stone->Remove();
    }

    timer_playing=0;
}

void gs_playing::load_level(std::string level_filename)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result=doc.load_file(("Data/"+level_filename).c_str());
    if(!result)
    {
        std::cout<<"XML parsed with errors, attr value: ["<<doc.child("node").attribute("attr").value()<<"]\n";
        std::cout<<"Error description: "<<result.description()<<"\n";
        std::cout<<"Error offset: "<<result.offset<<" (error at [..."<<(result.offset)<<"]\n\n";
    }

    float player_pos_x=0;
    float player_pos_y=0;
    float player_pos_z=0;

    for(auto& c:doc.children())
    {
        for(pugi::xml_node& child:c.children())
        {
            std::string name(child.name());
            if(name=="static_model")
            {
                float pos_x=0;
                float pos_y=0;
                float pos_z=0;
                std::string name;

                for(pugi::xml_attribute& attr:child.attributes())
                {
                    if(std::string(attr.name())=="name")
                        name=attr.value();
                    else if(std::string(attr.name())=="pos_x")
                    {
                        std::string str(attr.value());
                        pos_x=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_y")
                    {
                        std::string str(attr.value());
                        pos_y=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_z")
                    {
                        std::string str(attr.value());
                        pos_z=std::stof(str);
                    }
                }

                if(name.size())
                {
                    Node* boxNode_=globals::instance()->scene->CreateChild();
                    nodes.push_back(boxNode_);
                    boxNode_->SetPosition(Vector3(pos_x,pos_y,pos_z));
                    StaticModel* boxObject=boxNode_->CreateComponent<StaticModel>();
                    set_model(boxObject,globals::instance()->cache,name);
                    boxObject->SetCastShadows(true);
                    boxObject->SetOccludee(true);
                    boxObject->SetOccluder(true);

                    float min_y=boxObject->GetWorldBoundingBox().min_.y_;
                    if(level_min_height>min_y)
                        level_min_height=min_y;

                    RigidBody* body=boxNode_->CreateComponent<RigidBody>();
                    body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
                    CollisionShape* shape=boxNode_->CreateComponent<CollisionShape>();
                    shape->SetTriangleMesh(globals::instance()->cache->GetResource<Model>(String(name.c_str(),name.size())+".mdl"));

                    globals::instance()->physical_world->SetGravity(Vector3(0,-9.81*4,0));
                }
            }
            else if(name=="flag")
            {
                float pos_x=0;
                float pos_y=0;
                float pos_z=0;

                for(pugi::xml_attribute& attr:child.attributes())
                {
                    if(std::string(attr.name())=="pos_x")
                    {
                        std::string str(attr.value());
                        pos_x=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_y")
                    {
                        std::string str(attr.value());
                        pos_y=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_z")
                    {
                        std::string str(attr.value());
                        pos_z=std::stof(str);
                    }
                }

                flag_positions.emplace_back(pos_x,pos_y,pos_z);
            }
            else if(name=="torch")
            {
                float pos_x=0;
                float pos_y=0;
                float pos_z=0;

                for(pugi::xml_attribute& attr:child.attributes())
                {

                    if(std::string(attr.name())=="pos_x")
                    {
                        std::string str(attr.value());
                        pos_x=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_y")
                    {
                        std::string str(attr.value());
                        pos_y=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_z")
                    {
                        std::string str(attr.value());
                        pos_z=std::stof(str);
                    }
                }

                torch_positions.emplace_back(pos_x,pos_y,pos_z);
            }
            else if(name=="player")
            {
                for(pugi::xml_attribute& attr:child.attributes())
                {
                    if(std::string(attr.name())=="pos_x")
                    {
                        std::string str(attr.value());
                        player_pos_x=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_y")
                    {
                        std::string str(attr.value());
                        player_pos_y=std::stof(str);
                    }
                    else if(std::string(attr.name())=="pos_z")
                    {
                        std::string str(attr.value());
                        player_pos_z=std::stof(str);
                    }
                }
            }
        }
    }

    player_.reset(new player(Vector3(player_pos_x,player_pos_y,player_pos_z),this));

    {   // "load" flags
        for(auto p:flag_positions)
        {
            Node* n=globals::instance()->scene->CreateChild("Flag");
            nodes.push_back(n);

            n->SetPosition(p);
            StaticModel* boxObject=n->CreateComponent<StaticModel>();
            set_model(boxObject,globals::instance()->cache,"Data/Models/flag");
            boxObject->SetCastShadows(true);
            flag_nodes.push_back(n);
        }

        for(auto p:torch_positions)
            spawn_torch(p);
    }
}

void gs_playing::update(StringHash eventType,VariantMap& eventData)
{
    if(globals::instance()->game_states.size()>1)
        return;

    Input* input=GetSubsystem<Input>();
    float timeStep=eventData[Update::P_TIMESTEP].GetFloat();
    timer_playing+=timeStep;

    // check if there should be rocks spawned due to a distance trigger
    if(!rocks_spawned&&(player_->node->GetPosition()-Vector3(-200,37,-125)).Length()<150)
    {
        rocks_spawned=true;
        for(int i=0;i<100;i++)
        {
            auto node_stone=globals::instance()->scene->CreateChild("Stone");
            nodes.push_back(node_stone);
            StaticModel* boxObject=node_stone->CreateComponent<StaticModel>();
            boxObject->SetModel(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
            boxObject->SetMaterial(globals::instance()->cache->GetResource<Material>("Materials/rock.xml"));
            boxObject->SetCastShadows(true);
            float s=1.0+Random(3.0f);
            node_stone->SetScale(s);
            boxObject->SetOccludee(true);
            boxObject->SetDrawDistance(200);
            boxObject->SetShadowDistance(200);

            PhysicsRaycastResult result;
            Vector3 pos(-140-Random(80.0f),40,-80-Random(70.0f));
            Ray ray(pos,Vector3(0,-1,0));
            globals::instance()->physical_world->SphereCast(result,ray,2,100);
            if(result.distance_<=1000)
                pos=result.position_+Vector3(0,5,0);

            auto body_stone=node_stone->CreateComponent<RigidBody>();
            body_stone->SetPosition(pos);
            body_stone->SetCollisionLayer(2);
            body_stone->SetMass(50.0*s*s);
            body_stone->SetLinearDamping(0.2f);
            body_stone->SetAngularDamping(0.2f);
            //body_stone->SetAngularFactor(Vector3(0,1,0));
            body_stone->SetFriction(0.6);
            CollisionShape* shape=node_stone->CreateComponent<CollisionShape>();
            //shape->SetCapsule(1,1.2);
            shape->SetConvexHull(globals::instance()->cache->GetResource<Model>("Models/rock.mdl"));
        }
    }
std::string str;
    {
        static double last_second=0;
        static double last_second_frames=1;
        static timer this_second;
        static double this_second_frames=0;
        this_second_frames++;
        if(this_second.until_now()>=1)
        {
            last_second=this_second.until_now();
            last_second_frames=this_second_frames;
            this_second.reset();
            this_second_frames=0;
        }

        if(last_second!=0)
            str.append(std::to_string(last_second_frames/last_second).substr(0,6));
        str.append(" FPS   Position: ");
        str.append(std::to_string(player_->node->GetPosition().x_).substr(0,6));
        str.append(", ");
        str.append(std::to_string(player_->node->GetPosition().y_).substr(0,6));
        str.append(", ");
        str.append(std::to_string(player_->node->GetPosition().z_).substr(0,6));
        str.append("\nLevel Time: ");

        if(goal_time>0)
            str.append(std::to_string(goal_time));
        else
            str.append(std::to_string(timer_playing));

        str.append("s\nRemaining Flags: ");
        str.append(std::to_string(flag_nodes.size()));
        str.append("/");
        str.append(std::to_string(flag_positions.size()));
        if(goal_time>0)
            str.append("\nFinished!");

        String s(str.c_str(),str.size());
        text_->SetText(s);
    }

    player_->update(input,timeStep);

    Vector3 player_pos=player_->node->GetPosition();
    for(int i=0;i<flag_nodes.size();i++)
    {
        auto n=flag_nodes[i];
        n->Yaw(64*timeStep);
        if((player_pos-n->GetPosition()).Length()<2)
        {
            flag_nodes.erase(flag_nodes.begin()+i);
            n->Remove();
            for(int j=0;j<nodes.size();j++)
                if(nodes[j]==n)
                {
                    nodes.erase(nodes.begin()+j);
                    break;
                }

            if(flag_nodes.size()==0)
            {
                goal_time=timer_playing;
                auto e=new gs_level_end;
                std::string str="You collected all flags!\nNeeded time: "+std::to_string(goal_time)+"s";
                e->text_finished->SetText(str.c_str());
                globals::instance()->game_states.emplace_back(e);
            }
            break;
        }
    }

    if(player_->node->GetWorldPosition().y_<level_min_height-10)    // die if below level geometry
        globals::instance()->game_states.emplace_back(new gs_death);
}

void gs_playing::HandleKeyDown(StringHash eventType,VariantMap& eventData)
{
    if(globals::instance()->game_states.size()>1)
        return;
    using namespace KeyDown;
    int key=eventData[P_KEY].GetInt();
    if(key==KEY_ESC)
        globals::instance()->game_states.emplace_back(new gs_pause);

    if(key==KEY_L)
        spawn_torch(player_->node->GetPosition()+Vector3(2,1.9,0));
    if(key==KEY_V)
        camera_first_person=!camera_first_person;
}

void gs_playing::spawn_torch(Vector3 pos)
{
    Node* node=globals::instance()->scene->CreateChild("Light");
    nodes.push_back(node);

    PhysicsRaycastResult result;
    Ray ray(pos,Vector3(0,-1,0));
    globals::instance()->physical_world->SphereCast(result,ray,0.4,10);
    if(result.distance_<=10)
        pos=result.position_+Vector3(0,0.2,0);
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
    light->SetBrightness(1.5);
    light->SetColor(Color(2.0,1.2,.8,1.0));
    light->SetCastShadows(true);
    light->SetShadowDistance(300);
    light->SetDrawDistance(300);

    auto body_stone=node->CreateComponent<RigidBody>();
    body_stone->SetCollisionLayer(2);
    body_stone->SetMass(50.0);
    body_stone->SetLinearDamping(0.2f);
    body_stone->SetAngularDamping(0.2f);
    body_stone->SetFriction(0.6);
    CollisionShape* shape=node->CreateComponent<CollisionShape>();
    shape->SetBox(Vector3(0.7,1.47,0.7),Vector3(0,1.47/2,0));

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
    sound_torch_source->SetFrequency(sound_torch->GetFrequency()*Random(0.7f,1.3f));
}
