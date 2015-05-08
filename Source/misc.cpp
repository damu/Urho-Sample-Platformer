#include <fstream>

#include "misc.h"

using namespace std;
using namespace Urho3D;

void set_model(Urho3D::StaticModel* model,Urho3D::ResourceCache* cache,std::string filename)
{
    string filename_model=filename;
    model->SetModel(cache->GetResource<Model>(Urho3D::String(filename_model.append(".mdl").c_str())));
    string filename_txt=filename;
    filename_txt.append(".txt");
    ifstream file(filename_txt);
    string line;
    if(file.is_open())
    {
        int i=0;
        while(getline(file,line))
        {
            model->SetMaterial(i,cache->GetResource<Material>(Urho3D::String(line.c_str())));
            i++;
        }
    }
}
