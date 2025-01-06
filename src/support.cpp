#include "support.hpp"
#include <iostream>

long GetTime(){
    return std::chrono::system_clock::now().time_since_epoch().count();
}
