#include "dokan.h"
#include "dokanfs_msg.h"
#include "cache.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DokanFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, DokanFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, DokanFastIoQueryNetworkOpenInfo)
#endif

BufferCache basicInfoCache(sizeof(FILE_BASIC_INFORMATION));
BufferCache standardInfoCache(sizeof(FILE_STANDARD_INFORMATION));
BufferCache networkOpenInfoCache(sizeof(FILE_NETWORK_OPEN_INFORMATION));


extern "C" {
    void addStandardInfoCache(PFILE_OBJECT fileObject,
        PFILE_STANDARD_INFORMATION standardInformation) {
        if (fileObject && fileObject->FsContext2 &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

            standardInfoCache.addCache(
                ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName,
                standardInformation);
        }
    }

    void addBasicInfoCache(PFILE_OBJECT fileObject,
        PFILE_BASIC_INFORMATION standardInformation) {
        if (fileObject && fileObject->FsContext2 &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {
            basicInfoCache.addCache(
                ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName,
                standardInformation);
        }
    }

    void addNetworkOpenInfoCache(PFILE_OBJECT fileObject,
        PFILE_NETWORK_OPEN_INFORMATION information) {
        if (fileObject && fileObject->FsContext2 &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

            networkOpenInfoCache.addCache(
                ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, information);
        }
    }


    BOOLEAN DokanFastIoQueryBasicInfo(_In_ struct _FILE_OBJECT *fileObject, _In_ BOOLEAN Wait,
        _Out_ PFILE_BASIC_INFORMATION buffer, _Out_ PIO_STATUS_BLOCK ioStatus,
        _In_ struct _DEVICE_OBJECT *DeviceObject){
        if (fileObject && fileObject->FsContext2 &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

            return basicInfoCache.getCache(
                ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName,
                buffer, ioStatus);
        }
        return FALSE;
    }

    BOOLEAN DokanFastIoQueryStandardInfo(_In_ struct _FILE_OBJECT *fileObject,
                                         _In_ BOOLEAN Wait,
                                         _Out_ PFILE_STANDARD_INFORMATION buffer,
                                         _Out_ PIO_STATUS_BLOCK ioStatus,
                                         _In_ struct _DEVICE_OBJECT *DeviceObject) {
        if (fileObject && fileObject->FsContext2 &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

            return standardInfoCache.getCache(
                ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, buffer, ioStatus);
        }
        return FALSE;
    }

    BOOLEAN DokanFastIoQueryNetworkOpenInfo(_In_ struct _FILE_OBJECT *fileObject,
                                            _In_ BOOLEAN Wait,
                                            _Out_ PFILE_NETWORK_OPEN_INFORMATION buffer,
                                            _Out_ PIO_STATUS_BLOCK ioStatus,
                                            _In_ struct _DEVICE_OBJECT *DeviceObject) {
        if (fileObject && fileObject->FsContext2 &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb &&
            ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName.Buffer) {

            return networkOpenInfoCache.getCache(
                ((PDokanCCB)(fileObject->FsContext2))->Fcb->FileName, buffer, ioStatus);
        }
        return FALSE;
    }
}