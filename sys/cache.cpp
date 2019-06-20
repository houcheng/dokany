#include "cache.h"
#include "dokan.h"

// Time and period are in unit of 100 ns
#define TIMEOUT (2*10000000)
#define CLEANUP_ACTIVITY_COUNT  1000
#define CLEANUP_MIN_PERIOD      (5*10000000)

BufferCache::BufferCache(int bufferLen) {
  this->bufferLen = bufferLen;
  ExInitializeFastMutex(&mutex);
}

BufferCache::~BufferCache() {
  for (auto it = cache.first(); it;) {
    free(it->data.buffer);

    auto key = it->key;
    it = it->next();
    cache.del(key);
  }
}

void BufferCache::addCache(UNICODE_STRING &unicodeString, void *buffer) {
  wstring filename(unicodeString);
  addCache(&filename, buffer);
}

// Adds or update cached buffer
void BufferCache::addCache(wstring *filename, void *buffer) {
  if (KeGetCurrentIrql() > APC_LEVEL)
    return;

  ExAcquireFastMutex(&mutex);
  if (cache.contains(filename)) {
    // Removes old cache item
    InfoCacheEntry entry = {0};
    cache.get(filename, &entry);
    if (entry.buffer)
      free(entry.buffer);

    cache.del(filename);
  }

  // Adds cache item
  InfoCacheEntry entry = {0};
  KeQuerySystemTime(&entry.time);
  entry.buffer = clone(buffer, bufferLen);
  cache.add(filename, entry);

  // DbgPrint("Add cache for file: %wZ\n", &filename->content);
  activityCount++;

  ExReleaseFastMutex(&mutex);
}

BOOL BufferCache::getCache(UNICODE_STRING &unicodeString, void *buffer, PIO_STATUS_BLOCK IoStatus) {
  wstring filename(unicodeString);
  return getCache(&filename, buffer, IoStatus);
}

BOOL BufferCache::getCache(wstring *filename, void *buffer,
                           PIO_STATUS_BLOCK IoStatus) {
  if (KeGetCurrentIrql() > APC_LEVEL)
    return FALSE;

  ULONG now;
  KeQuerySystemTime(&now);

  __try {
    ExAcquireFastMutex(&mutex);
    if (cache.contains(filename)) {
      InfoCacheEntry entry = {0};
      cache.get(filename, &entry);

      if ((now - entry.time) > TIMEOUT) {
          cache.del(filename);
          __leave;
      }

      if (entry.buffer) {
        RtlCopyMemory(buffer, entry.buffer, bufferLen);

        if (IoStatus) {
          IoStatus->Status = STATUS_SUCCESS;
        }
        // DbgPrint("Reply basic info from cache\n");
        return TRUE;
      }
    }
  } __finally {
      activityCount++;
      if (activityCount > CLEANUP_ACTIVITY_COUNT && (now - lastCleanupTime) > CLEANUP_MIN_PERIOD)
          cleanupCache();
      ExReleaseFastMutex(&mutex);
  }
  return FALSE;
}

void BufferCache::cleanupCache() {
    activityCount = 0;
    lastCleanupTime = 0;

    ULONG now;
    KeQuerySystemTime(&now);
    for (auto it = cache.first(); it;) {
        if ((now - it->data.time) < TIMEOUT) {
            it = it->next();
            continue;
        }

        free(it->data.buffer);

        auto key = it->key;
        it = it->next();
        cache.del(key);
    }
}


BufferCache basicInfoCache(sizeof(FILE_BASIC_INFORMATION));
BufferCache standardInfoCache(sizeof(FILE_STANDARD_INFORMATION));
BufferCache networkOpenInfoCache(sizeof(FILE_NETWORK_OPEN_INFORMATION));

// TODO: removed in future
extern "C" void initCache() { 
    initCppSupport();
}

extern "C" void addStandardInfoCache(PFILE_OBJECT fileObject,
                                     PFILE_STANDARD_INFORMATION standardInformation) {
  if (fileObject && fileObject->FsContext2 &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer ) {

      standardInfoCache.addCache(
          ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, 
          standardInformation);
  }
}

extern "C" void addBasicInfoCache(PFILE_OBJECT fileObject,
                                  PFILE_BASIC_INFORMATION standardInformation) {
  if (fileObject && fileObject->FsContext2 &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {
    basicInfoCache.addCache(
        ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName,
        standardInformation);
  }
}

extern "C" void addNetworkOpenInfoCache(PFILE_OBJECT fileObject,
                     PFILE_NETWORK_OPEN_INFORMATION information) {
  if (fileObject && fileObject->FsContext2 &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

    networkOpenInfoCache.addCache(
        ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, information);
  }
}

extern "C" BOOL getBasicInfoCache(PFILE_OBJECT fileObject,
                                  PFILE_BASIC_INFORMATION buffer,
                                  PIO_STATUS_BLOCK ioStatus) {
  if (fileObject && fileObject->FsContext2 &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {
    
    return basicInfoCache.getCache(
        ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, 
        buffer, ioStatus);
  }
  return FALSE;
}

extern "C" BOOL getStandardInfoCache(PFILE_OBJECT fileObject,
                                PFILE_STANDARD_INFORMATION buffer,
                                PIO_STATUS_BLOCK ioStatus) {
  if (fileObject && fileObject->FsContext2 &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

    return standardInfoCache.getCache(
        ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, buffer, ioStatus);
  }
  return FALSE;
}

extern "C" BOOL getNetworkOpenInfoCache(PFILE_OBJECT fileObject,
                                     PFILE_NETWORK_OPEN_INFORMATION buffer,
                                     PIO_STATUS_BLOCK ioStatus) {
  if (fileObject && fileObject->FsContext2 &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
      ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

    return networkOpenInfoCache.getCache(
        ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, buffer, ioStatus);
  }
  return FALSE;
}
