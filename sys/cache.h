#include "dokan.h"
#include "CppSupport.h"
#include "hash.h"
#include "wstring.h"

struct InfoCacheEntry {
  ULONG64 time;
  void *buffer;
};

class NONPAGESECTION BufferCache {
    int bufferLen = 0;
    FAST_MUTEX mutex = {0};
    KeyHash<InfoCacheEntry> cache;

public:
    BufferCache(int bufferLen);
    ~BufferCache();

    void addCache(UNICODE_STRING &unicodeString, void *buffer);
    void addCache(wstring *filename, void *buffer);
    BOOL getCache(UNICODE_STRING &unicodeString, void *buffer,
                               PIO_STATUS_BLOCK IoStatus);
    BOOL getCache(wstring *filename, void *buffer, PIO_STATUS_BLOCK IoStatus);
};
