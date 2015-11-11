#ifndef GS_MAIN_MENU_H
#define GS_MAIN_MENU_H

#include "game_state.h"

#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Skybox.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>

/// The main menu displayed when starting the game.
class gs_main_menu : public game_state
{
public:
    Urho3D::Node* node_rotating_flag;
    Urho3D::Window* window_menu;
    Urho3D::ListView* lv_levels;
    map_times highscores;

    gs_main_menu();
    void update(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandlePlayPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleKeyDown(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleClosePressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData){globals::instance()->engine->Exit();}

    virtual const Urho3D::String& GetTypeName() const override {static Urho3D::String name("gs_main_menu");return name;}   // this could be correct
};

#endif // GS_MAIN_MENU_H
