#include <reporter.h>
#include <vector>

static std::vector<const char *> names;
static std::vector<void *> bindings;

// 0 - bool
// 1 - int
// 2 - dbl
// 3 - str
static std::vector<char> types;




void reporter::bind_bool(const char * name, bool * value){
    names.push_back(name);
    bindings.push_back((void*) value);
    types.push_back(0);
}


void reporter::bind_int(const char * name, int * value){
    names.push_back(name);
    bindings.push_back((void*) value);
    types.push_back(1);
}


void reporter::bind_dbl(const char * name, double * value){
    names.push_back(name);
    bindings.push_back((void*) value);
    types.push_back(2);
}


void reporter::bind_str(const char * name, std::string * str){
    names.push_back(name);
    bindings.push_back((void*) str);
    types.push_back(3);
}

std::string reporter::get_report(){
    std::string out = "";
    for(int i = 0; i < names.size(); i ++){
        switch(types[i]){
            case 0:
                out += std::to_string(*((bool *) (bindings[i])));
                break;
            case 1:
                out += std::to_string(*((int *) (bindings[i])));
                break;
            case 2:
                out += std::to_string(*((double *) (bindings[i])));
                break;
            case 3:
                out +=*((std::string *) bindings[i]);
                break;
            default:
                out += "null";
                break;
        }
        out += " ";
    }
    return out;
}

std::string reporter::get_json_report(){
    std::string out = "{";
    for(int i = 0; i < names.size(); i ++){
        std::string name(names[i]);
        out+=name+":";
        switch(types[i]){
            case 0:
                out += std::to_string(*((bool *) (bindings[i])));
                break;
            case 1:
                out += std::to_string(*((int *) (bindings[i])));
                break;
            case 2:
                out += std::to_string(*((double *) (bindings[i])));
                break;
            case 3:
                out += '"'+*((std::string *) bindings[i])+'"';
                break;
            default:
                out += "null";
                break;
        }
        out += ",";
    }
    return out+"}";
}

std::string reporter::get_keys() {
    std::string out = "";
    for(int i = 0; i < names.size(); i ++){
        out += names[i];
        out += " ";
    }
    return out;
}