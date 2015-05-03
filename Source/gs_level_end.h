#ifndef GS_LEVEL_END_H
#define GS_LEVEL_END_H

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
#include <Urho3D/UI/Font.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>

/// This one is stacked on top of the playing game mode after finishing a level to display statistics.
class gs_level_end : public game_state
{
public:
    Urho3D::Node* boxNode_;
    Urho3D::Window* window_;

    gs_level_end();
    void update(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandlePlayPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleKeyDown(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleClosePressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData){globals::instance()->engine->Exit();}

    virtual const Urho3D::String& GetTypeName() const {static Urho3D::String name("gs_level_end");return name;}   // this could be correct
};

#endif // GS_LEVEL_END_H
