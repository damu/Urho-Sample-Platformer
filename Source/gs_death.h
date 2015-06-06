#ifndef GS_DEATH
#define GS_DEATH

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

/// This one is stacked on top of the playing game mode when the player died.
class gs_death : public game_state
{
public:
    Urho3D::Window* window_;

    gs_death();
    void HandleRestartPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);
    void HandleMainMenuPressed(Urho3D::StringHash eventType,Urho3D::VariantMap& eventData);

    virtual const Urho3D::String& GetTypeName() const {static Urho3D::String name("gs_death");return name;}   // this could be correct
};

#endif // GS_DEATH
