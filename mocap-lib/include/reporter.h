#ifndef REPORTER_H
#define REPORTER_H
#include <string>
// #include <string_view>

namespace reporter {
    // void set_connection()

    void bind_str(const char * name, std::string * str);
    void bind_bool(const char * name, bool * value);
    void bind_int(const char * name, int * value);
    void bind_dbl(const char * name, double * value);

    std::string get_report();
    std::string get_keys();
    std::string get_json_report();
};





#endif