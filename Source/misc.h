#ifndef MISC_H
#define MISC_H

#include <string>
#include <memory>
#include <vector>
#include <sys/time.h>
#include <iostream>
#include <fstream>

#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>

// copied from my source collection.
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

/// \brief Calls SetModel on the given model and tries to load the model file and all texture files mentioned in a model_name+".txt".
/// model_name is supposed to have no file extension. Example: "Data/Models/Box", loads the model "Data/Models/Box.mdl".
template<typename T>
void set_model(T* model,Urho3D::ResourceCache* cache,std::string model_name)
{
    std::string filename_model=model_name;
    model->SetModel(cache->GetResource<Urho3D::Model>(Urho3D::String(filename_model.append(".mdl").c_str())));
    std::string filename_txt=model_name;
    filename_txt.append(".txt");
    std::ifstream file(filename_txt);
    std::string line;
    if(file.is_open())
    {
        int i=0;
        while(getline(file,line))
        {
            model->SetMaterial(i,cache->GetResource<Urho3D::Material>(Urho3D::String(line.c_str())));
            i++;
        }
    }
}

#endif // MISC_H
