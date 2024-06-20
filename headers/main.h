#ifndef MAIN_H
#define MAIN_H

#ifdef MAKE_DEBUG

#include <iostream>
#include "Hexdump.hpp"
#define DBG(X) {std::cout << X << std::endl;}

#else

#define DBG(X) {}

#endif


#ifdef _WIN32
#define POCO_STATIC 1		   
#pragma comment(lib, "ws2_32") 
#pragma comment(lib, "IPHLPAPI.lib")
#endif


#endif
