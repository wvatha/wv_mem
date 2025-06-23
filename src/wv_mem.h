#ifndef _WV_MEM_H
#define _WV_MEM_H

#if defined(WIN32) || defined(_WIN32)
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#include <windows.h>
#elif defined(__linux__) || defined(__linux)
#include <sys/mman.h>
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT __attribute__((visibility("default")))
#elif defined(__APPLE__) || defined(__MACH__)
#include <sys/mman.h>
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT __attribute__((visibility("default")))
#endif // OS checks
#include <stddef.h>

DLL_EXPORT
void* wv_allocate_huge(size_t size);

DLL_EXPORT
void* wv_allocate_page(size_t size);

DLL_EXPORT
void wv_free(void* ptr, size_t size);


#endif // _WV_MEM_H