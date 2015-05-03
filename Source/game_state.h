#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <memory>
#include <vector>
#include <sys/time.h>
#include <iostream>

#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/UIElement.h>

class game_state;

/// For stuff you want to share across game states.
class globals
{
public:
    Urho3D::Camera* camera=0;
    Urho3D::Scene* scene=0;
    Urho3D::ResourceCache* cache=0;
    Urho3D::Context* context=0;
    Urho3D::UIElement* ui_root=0;
    Urho3D::Engine* engine=0;

    /// The current game states so that game states can switch to another game state.
    /// Watch out that changing a game state will immedietly delete the current one.
    /// Also game states can be stacked on top of each other so they run paralell. This is used for the pause mode.
    std::vector<std::unique_ptr<game_state>> game_states;

    /// Meyer Singleton
    static globals* instance()
    {
        static globals g;
        return &g;
    }
};

class game_state : public Urho3D::Object
{
public:
    std::vector<Urho3D::Node*> nodes;   ///< Put all your nodes here so that they are removed on game state deletion.
    std::vector<Urho3D::UIElement*> gui_elements;   ///< Put all your nodes here so that they are removed on game state deletion.
    // there could be other vectors here with stuff that will be automatically removed in the destructor

    game_state() : Urho3D::Object(globals::instance()->context) {}
    /// Removes all GUI elements amd all nodes in the ´nodes´ vector.
    virtual ~game_state()
    {
        for(auto e:gui_elements)
            e->Remove();
        gui_elements.clear();
        for(auto n:nodes)
            n->Remove();
        nodes.clear();
    }

    // these three functions need be here because of Urho3D::Object
    virtual Urho3D::StringHash GetType() const {return GetTypeName();}       // no idea if this is as supposed to be, but it works
    virtual Urho3D::StringHash GetBaseType() const {return GetTypeName();}   // no idea if this is as supposed to be, but it works
    virtual const Urho3D::String& GetTypeName() const {static Urho3D::String name("game_state");return name;}   // this could be correct
};

// copied from my source collection. Maybe this should be moved elsewhere.
/**
 * @brief The timer class can be used to measure times and optionaly output them automatically on destruction.
 * Example:
 * \code
 * {
 *   timer _("test");
 *   sleep(0.1);
 * }                        // the timer is destructed here as it goes out of scope and prints something like "0.100132 <- test"
 * \endcode
 */
class timer
{
public:
    std::string str;        ///< the message printed, after the measured time, on destruction
    timeval tv;             ///< the timeval the timer started on
    bool output;            ///< if the timer should print something on destruction

    timer(std::string str,bool output=true) : str(str),tv(),output(output){reset();}
    timer() : str(""),tv(),output(false){reset();}
    ~timer()
    {
        if(!output)
            return;
        timeval end;
        gettimeofday(&end,NULL);
        std::cout<<(((end.tv_sec-tv.tv_sec)*1000000.0+(end.tv_usec-tv.tv_usec))/1000000.0)<<" <- "<<str<<std::endl;
    }

    /// resets the timer by putting the current time into the member /tv/
    void reset(){gettimeofday(&tv,0);}

    /// returns the time which has passed since starting the timer
    double until_now() const
    {
        timeval end;
        gettimeofday(&end,NULL);
        return(((end.tv_sec-tv.tv_sec)*1000000.0+(end.tv_usec-tv.tv_usec))/1000000.0);
    }

    /// same as until_now()
    operator double() const {return until_now();}
};


#endif // GAME_STATE_H
