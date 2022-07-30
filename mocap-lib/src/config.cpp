
#include <config.h>
// #include <rapidjson/document.h>
#include <json.hpp>
#include <fstream>
#include <string>
#include <logger.h>
#include <settings.h>
#include <mutex>
// #include <mutex.h>


static nlohmann::json configuration;
static bool loaded = false;

std::mutex config_mutex;

int index_of(const char * str, char c, int start = 0) {
    for(int i = start; str[i] != '\0'; i++){
        if(str[i] == c) {
            return i;
        }
    }
    return -1;
}

void substr(char * out, const char * bruh, int a, int b){
    int k = 0;
    for(int i = a; i != b && bruh[i] != '\0'; i ++){
        out[k++] = bruh[i];
    }
    out[k] = '\0';
}

int substr_chr(char * out, const char * bruh, char c, int a, int b){
    int k = 0;
    for(int i = a; i != b && bruh[i] != '\0'; i ++){
        if(bruh[i] == c){
            out[k++] = '\0';
            return i;
        }
        out[k++] = bruh[i];
    }
    out[k] = '\0';
    return -1;
}


// static char * filename;
void config::load_file(const char * filename){
    std::lock_guard<std::mutex> config_lock(config_mutex);
    std::ifstream in(filename);
    if(in){
        std::string str;
        std::getline(in, str, '\0');
        
        configuration = nlohmann::json::parse(str);
    }
}

void config::write_to_file(const char * filename){
    std::lock_guard<std::mutex> config_lock(config_mutex);

    std::ofstream out(filename);    
    if(out){
        logger::info("Saving configuration to \"{}\".", filename);
        out << configuration;
        out.close();
    }else{
        logger::warn("Couldn't create configuration file\n.");
    }
}

void config::load_file(){
    load_file(DEFAULT_CONFIG_FILE);
}

void config::write_to_file(){
    write_to_file(DEFAULT_CONFIG_FILE);
}





char buf[100];
int config::get_config_int(const char * name, int dft){
    std::lock_guard<std::mutex> config_lock(config_mutex);
    
    nlohmann::json *curr = &configuration;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<int>();
    }

    (*curr)[buf]=dft;
    return dft;
}

double config::get_config_dbl(const char * name, double dft){
    std::lock_guard<std::mutex> config_lock(config_mutex);
    
    nlohmann::json *curr = &configuration;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<double>();
    }

    (*curr)[buf]=dft;
    return dft;
}

std::string config::get_config_str(const char * name, std::string dft){
    std::lock_guard<std::mutex> config_lock(config_mutex);
    
    nlohmann::json *curr = &configuration;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<std::string>();
    }

    (*curr)[buf]=dft;
    return dft;
}



bool config::get_config_bool(const char * name, bool dft){
    std::lock_guard<std::mutex> config_lock(config_mutex);
    
    nlohmann::json *curr = &configuration;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<bool>();
    }

    (*curr)[buf]=dft;
    return dft;
}

// void config::set_config(const char * name, int value){
//     std::lock_guard<std::mutex> config_lock(config_mutex);
//     configuration[name]=value;
// }

// void config::set_config(const char * name, double value){
//     std::lock_guard<std::mutex> config_lock(config_mutex);
//     configuration[name]=value;
// }


config::config::config() {
    base = nlohmann::json();   
}

config::config::config(const char * filename) {
    std::ifstream in(filename);
    if(in){
        std::string str;
        std::getline(in, str, '\0');
        base = nlohmann::json::parse(str);
    }
}



int config::config::get_config_int(const char * name, int dft){
    char buf[100];
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<int>();
    }

    (*curr)[buf]=dft;
    return dft;
}

double config::config::get_config_dbl(const char * name, double dft){
    char buf[100];
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<double>();
    }

    (*curr)[buf]=dft;
    return dft;
}

std::string config::config::get_config_str(const char * name, std::string dft){
    char buf[100];
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<std::string>();
    }

    (*curr)[buf]=dft;
    return dft;
}



bool config::config::get_config_bool(const char * name, bool dft){
    char buf[100];
    
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);

    if(curr->contains(buf)){
        return (*curr)[buf].get<bool>();
    }

    (*curr)[buf]=dft;
    return dft;
}


void config::config::write_to_file(const char * filename){
    std::ofstream out(filename);    
    if(out){
        logger::info("Saving configuration to \"{}\".", filename);
        out << configuration;
        out.close();
    }else{
        logger::warn("Couldn't create configuration file\n.");
    }
}

void config::config::set(const char * name, bool value){
    char buf[100];
    
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);
    (*curr)[buf]=value;
}


void config::config::set(const char * name, int value){
    char buf[100];
    
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);
    (*curr)[buf]=value;
}


void config::config::set(const char * name, double value){
    char buf[100];
    
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);
    (*curr)[buf]=value;
}


void config::config::set(const char * name, std::string value){
    char buf[100];
    
    nlohmann::json *curr = &base;
    int i = substr_chr(buf, name, '.', 0, -1);
    int l = 0;
    while(i >= 0){
        // logger::info("{}, {}", curr.dump(), buf);
        // logger::info("{}", i);
        l = i;
        curr = &((*curr)[buf]);
        i = substr_chr(buf, name, '.', i+1, -1);
    }
    substr(buf, name, l+1, -1);
    (*curr)[buf]=value;
}