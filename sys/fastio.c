#include "dokan.h"

#include "dokanfs_msg.h"

BOOLEAN DokanFastIoQueryBasicInfo(_In_ struct _FILE_OBJECT *FileObject,
                                  _In_ BOOLEAN Wait,
                                  _Out_ PFILE_BASIC_INFORMATION Buffer,
                                  _Out_ PIO_STATUS_BLOCK IoStatus,
                                  _In_ struct _DEVICE_OBJECT *DeviceObject);
BOOLEAN DokanFastIoQueryStandardInfo(_In_ struct _FILE_OBJECT *FileObject,
                                     _In_ BOOLEAN Wait,
                                     _Out_ PFILE_STANDARD_INFORMATION Buffer,
                                     _Out_ PIO_STATUS_BLOCK IoStatus,
                                     _In_ struct _DEVICE_OBJECT *DeviceObject);
BOOLEAN DokanFastIoQueryNetworkOpenInfo(_In_ struct _FILE_OBJECT *FileObject,
                                        _In_ BOOLEAN Wait, 
                                        _Out_ PFILE_NETWORK_OPEN_INFORMATION Buffer,
                                        _Out_ PIO_STATUS_BLOCK IoStatus, 
                                        _In_ struct _DEVICE_OBJECT *DeviceObject);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DokanFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, DokanFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, DokanFastIoQueryNetworkOpenInfo)
#endif

BOOLEAN DokanFastIoQueryBasicInfo(_In_ struct _FILE_OBJECT *FileObject, _In_ BOOLEAN Wait,
    _Out_ PFILE_BASIC_INFORMATION Buffer, _Out_ PIO_STATUS_BLOCK IoStatus,
    _In_ struct _DEVICE_OBJECT *DeviceObject){
  return getBasicInfoCache(FileObject, Buffer, IoStatus);
}

BOOLEAN DokanFastIoQueryStandardInfo(_In_ struct _FILE_OBJECT *FileObject,
                                     _In_ BOOLEAN Wait,
                                     _Out_ PFILE_STANDARD_INFORMATION Buffer,
                                     _Out_ PIO_STATUS_BLOCK IoStatus,
                                     _In_ struct _DEVICE_OBJECT *DeviceObject) {
  return getStandardInfoCache(FileObject, Buffer, IoStatus);
}

BOOLEAN DokanFastIoQueryNetworkOpenInfo(_In_ struct _FILE_OBJECT *FileObject,
                                        _In_ BOOLEAN Wait,
                                        _Out_ PFILE_NETWORK_OPEN_INFORMATION Buffer,
                                        _Out_ PIO_STATUS_BLOCK IoStatus,
                                        _In_ struct _DEVICE_OBJECT *DeviceObject) {
  return getNetworkOpenInfoCache(FileObject, Buffer, IoStatus);
}