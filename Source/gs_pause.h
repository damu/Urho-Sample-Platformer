#ifndef GS_PAUSE_H
#define GS_PAUSE_H

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
#include <Urho3D/UI/Text.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>

/// Pauses Urho and is stacked on top of the playing game state and called by that when pressing the escape key.
class gs_pause : public game_state
{
public:
    Urho3D::Node* boxNode_;
    Urho3D::Window* window_;

    gs_pause();
    ~gs_pause();
    void HandleKeyDown(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleResumePressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleRestartPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleMainMenuPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleQuitPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData){globals::instance()->engine->Exit();}

    virtual const Urho3D::String& GetTypeName() const {static Urho3D::String name("gs_pause");return name;}   // this could be correct
};

#endif // GS_PAUSE_H
