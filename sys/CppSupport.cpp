#include "CppSupport.h"
#include <wdm.h>

//
// Compiler generate two dynamic functions for each global object, cc() and dc().
// cc() calls class constructor with pointer and calls atexit(dc).
// dc() calls class destructor with pointer.
//
// All cc() are linked in regions .crt$xca to .crt$xcz, and called by initCppSupport()
// The dc() are stored in exitList by atexit() and called by deleteCppGlobalObjects()
//

#define _CRTALLOC(x) __declspec(allocate(x))

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)

_CRTALLOC(".CRT$XCA") PVFV __crtXca[] = { NULL };
_CRTALLOC(".CRT$XCZ") PVFV __crtXcz[] = { NULL };

//
// Restores to DDK original data section
//
#pragma data_seg()
#pragma comment(linker, "/merge:.CRT=.rdata")

bool isCppInitialized = false;

//
// Stores dynamic generated destructor functions
//
typedef struct {
    LIST_ENTRY  link;
    PVFV         exitFunc;
} EXIT_FUNC_LIST, *PEXIT_FUNC_LIST;
LIST_ENTRY exitList;

void initCppSupport() {
    if (isCppInitialized) return;
    isCppInitialized = true;

    InitializeListHead(&exitList);

    for (PVFV *func = &__crtXca[0]; func < &__crtXcz[0]; func++) {
        if (*func) {
            (*func)();
        }
    }
}

void* __cdecl operator new(unsigned __int64 size) {
    void * ptr = malloc(size);
    return ptr;
}

void __cdecl operator delete(void *ptr, unsigned __int64 size) {
    UNREFERENCED_PARAMETER(size);
    free(ptr);
}

void free(void *ptr) {
    if (ptr != nullptr) {
        ExFreePool(ptr);
    }
}

void *cloneMemCopy(void *dst, void *src, size_t size) {
    RtlCopyMemory(dst, src, size);
    return dst;
}

//
// Learned from github htscpp
//
void deleteCppGlobalObjects() {
    PEXIT_FUNC_LIST pFuncListEntry;

    while (!IsListEmpty(&exitList)) {
        //
        // this cast relies on link being the first field of EXIT_FUNC_LIST.
        //
        pFuncListEntry = (PEXIT_FUNC_LIST)RemoveHeadList(&exitList);

        if (pFuncListEntry->exitFunc) {
            pFuncListEntry->exitFunc();
        }

        free(pFuncListEntry);
    }
}

int __cdecl atexit(PVFV func) {
    auto pFuncListEntry = (PEXIT_FUNC_LIST)malloc(sizeof(EXIT_FUNC_LIST));
    if (!pFuncListEntry) {
        return -1;
    }

    pFuncListEntry->exitFunc = func;
    InsertHeadList(&exitList, &pFuncListEntry->link);
    return 0;
}

