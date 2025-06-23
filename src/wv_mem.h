#ifndef _WV_MEM_H
#define _WV_MEM_H

#if defined(WIN32) || defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#include <windows.h>

DLL_EXPORT
void* wv_allocate_huge(size_t size) {
    // Large page allocation on Windows

    
}


#elif defined(__linux__) || defined(__linux)

#elif defined(__APPLE__) || defined(__MACH__)

#endif // OS checks

#endif // _WV_MEM_H