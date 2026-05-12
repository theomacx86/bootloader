#include "AutoGen.h"
#include "Library/BaseLib.h"
#include "Library/DebugLib.h"
#include "Library/UefiBootServicesTableLib.h"
#include "ProcessorBind.h"
#include "Protocol/DevicePath.h"
#include "Protocol/LoadedImage.h"
#include "Protocol/SimpleFileSystem.h"
#include "Uefi.h"
#include "Uefi/UefiBaseType.h"
#include "Uefi/UefiMultiPhase.h"
#include <Guid/FileInfo.h>
#include <Library/UefiLib.h>

#include "Include/Elf.h"
#include "Include/E820.h"

EFI_STATUS
EFIAPI
UefiMain(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {

  EFI_STATUS Status;
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
  EFI_FILE_PROTOCOL *Root = NULL;
  EFI_FILE_PROTOCOL *File = NULL;
  EFI_FILE_INFO * FileInfo = NULL;
  UINTN FileInfoSize = 0;
  UINTN KernelSize = 0;
  VOID * Kernel = NULL;

  Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid,
                               (VOID **)&LoadedImage);
  if (Status != EFI_SUCCESS) {
    Print(L"Could not open EFI_LOADED_IMAGE_PROTOCOL\n");
    return Status;
  }

  Status = gBS->HandleProtocol(LoadedImage->DeviceHandle,
                               &gEfiSimpleFileSystemProtocolGuid,
                               (VOID **)&FileSystem);
  if (Status != EFI_SUCCESS) {
    Print(L"Could not open EFI_SIMPLE_FILE_SYSTEM_PROTOCOL\n");
    return Status;
  }

  Status = FileSystem->OpenVolume(FileSystem, &Root);
  if (Status != EFI_SUCCESS) {
    Print(L"Could not open volume\r\n");
    return Status;
  }

  Status = Root->Open(Root, &File, L"\\chik", EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
  if (Status != EFI_SUCCESS) {
    Print(L"chik not found\r\n");
    return Status;
  }

  Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
  if(Status != EFI_BUFFER_TOO_SMALL)
  {
      return Status;
  }

  Status = gBS->AllocatePool(EfiLoaderData, FileInfoSize, (VOID**) &FileInfo);
  if(Status != EFI_SUCCESS)
  {
      Print(L"Failed to allocate file info buffer\r\n");
      return Status;
  }

  Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, FileInfo);
  if(Status != EFI_SUCCESS)
  {
      Print(L"Failed to query file info\r\n");
      return Status;
  }

  KernelSize = FileInfo->FileSize;
  Status = gBS->AllocatePool(EfiLoaderData, KernelSize, &Kernel);
  if(Status != EFI_SUCCESS)
  {
      Print(L"Failed to allocate kernel buffer %d bytes\r\n", FileInfo->FileSize);
      return Status;
  }

  Status = File->Read(File, &KernelSize, Kernel);
  if(Status != EFI_SUCCESS)
  {
      Print(L"Failed to read kernel file\r\n");
      return Status;
  }

  if(KernelSize != FileInfo->FileSize)
  {
      Print(L"File size mismatch\r\n");
      return EFI_UNSUPPORTED;
  }

  gBS->FreePool(FileInfo);

  Elf64_Ehdr * Header = NULL;
  Header = (Elf64_Ehdr *) Kernel;

  for(UINTN i = 0; i < SELFMAG; ++i)
  {
      if(Header->e_ident[i] != ELFMAG[i])
      {
          Print(L"Magic mismatch\r\n");
          return EFI_UNSUPPORTED;
      }
  }

  Print(L"Machine type is %d\r\n", Header->e_machine);

  gBS->FreePool(Kernel);
  return Status;
}
