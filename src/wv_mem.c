#include "wv_mem.h"

static int page_size = 0;

#if defined(WIN32) || defined(_WIN32)

#include <windows.h>

static int
check_page_size(void) {
    if (page_size == 0) {
        SYSTEM_INFO sys_info;
        GetSystemInfo(&sys_info);
        page_size = sys_info.dwPageSize;                                                                                                                                                     
    }
    return page_size;
}

static size_t
get_large_page_size(void) {
    static size_t large_page_size = 0;
    if (large_page_size == 0) {
        large_page_size = GetLargePageMinimum();
    }
    return large_page_size;
}

static int
wv_is_privileged(LPCTSTR privilege_name) {
    HANDLE h_token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &h_token)) {
        return 0;
    }

    LUID luid;
    if (!LookupPrivilegeValue(NULL, privilege_name, &luid)) {
        CloseHandle(h_token);
        return 0;
    }

    PRIVILEGE_SET ps = {
        .PrivilegeCount = 1,
        .Control = PRIVILEGE_SET_ALL_NECESSARY,
        .Privilege = { { .Luid = luid, .Attributes = SE_PRIVILEGE_ENABLED } }
    };

    BOOL has_priv = FALSE;
    if (!PrivilegeCheck(h_token, &ps, &has_priv)) {
        has_priv = FALSE;
    }

    CloseHandle(h_token);
    return has_priv ? 1 : 0;
}

static int
wv_enable_privilege(LPCTSTR privilege_name) {
    HANDLE h_token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &h_token)) {
        return 0;
    }

    LUID luid;
    if (!LookupPrivilegeValue(NULL, privilege_name, &luid)) {
        CloseHandle(h_token);
        return 0;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL success = AdjustTokenPrivileges(h_token, FALSE, &tp, sizeof(tp), NULL, NULL);
    DWORD err = GetLastError();
    CloseHandle(h_token);
    return success && err != ERROR_NOT_ALL_ASSIGNED;
}

static int
wv_is_se_lock_memory_enabled(void) {
    return wv_is_privileged(SE_LOCK_MEMORY_NAME);
}

static int
wv_enable_se_lock_memory(void) {
    return wv_is_se_lock_memory_enabled() || wv_enable_privilege(SE_LOCK_MEMORY_NAME);
}

DLL_EXPORT
void*
wv_allocate_page(size_t size) {
    if (size % check_page_size() != 0) {
        return NULL;
    }
    return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

DLL_EXPORT
void*
wv_allocate_huge(size_t size) {
    size_t lpsize = get_large_page_size();
    if (!wv_enable_se_lock_memory() || lpsize == 0 || size % lpsize != 0) {
        return wv_allocate_page(size);
    }

    void* ptr = VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
    if (!ptr) {
        return wv_allocate_page(size); // Fallback if large page allocation fails
    }
    return ptr;
}

DLL_EXPORT
void
wv_free(void* ptr, size_t size) {
    if (ptr) {
        VirtualFree(ptr, 0, MEM_RELEASE);
    }
}
#elif defined(__linux__) || defined(__linux)
#include <sys/mman.h>


DLL_EXPORT
void*
wv_allocate_page(size_t size) {
    if (size % getpagesize() != 0) {
        return NULL;
    }
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

DLL_EXPORT
void*
wv_allocate_huge(size_t size) {
    if (size % HugePageSize != 0) {
        return NULL;
    }
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
}

#endif
