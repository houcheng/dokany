#pragma once


#define SAFE_GUARD_TAG  'gtGS'
#define SAFE_GUARD_BUFER_TAG    'fbGS'

#ifdef _KERNEL_MODE

#include <fltKernel.h>

void* __cdecl operator new(unsigned __int64 size);
void __cdecl operator delete(void *ptr, unsigned __int64 size);

void *cloneMemCopy(void *dst, void *src, size_t size);

/**
 * Memory allocations of less than PAGE_SIZE are not necessarily page-aligned but are aligned
 * to 8-byte boundaries in 32-bit systems and to 16-byte boundaries in 64-bit systems.
 *
 * Note: Define it as macro is helpless as C++ objects calls new function that hides line
 * information
 */
inline void *malloc(int size) {
    return ExAllocatePoolWithTag(NonPagedPool, size, SAFE_GUARD_TAG);
}

inline void *clone(void *ptr, int size) {
    return cloneMemCopy(ExAllocatePoolWithTag(NonPagedPool, size, SAFE_GUARD_TAG), ptr, size);
}

inline void *mallocPaged(int size) {
    ExAllocatePoolWithTag(PagedPool, size, SAFE_GUARD_TAG);
}

#define ZALLOC(x, size) malloc(size); if(x) RtlZeroMemory((void *)x, size)

void free(void *ptr);

typedef (__cdecl *PVFV) (void);
int __cdecl atexit(
    PVFV func
);


//
// TODO: Checks each class has this __decl
//
#pragma warning(disable: 4091)
#define NONPAGESECTION __declspec(code_seg("$kerneltext$"))

extern "C" void initCppSupport();
extern "C" void deleteCppGlobalObjects();

#endif

