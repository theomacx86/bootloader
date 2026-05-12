#ifndef UEFI_H
#define UEFI_H

#include "ProcessorBind.h"
#include <Uefi.h>

#define E820_MAX 128

#define E820_RAM 1
#define E820_RESERVED 2
#define E820_ACPI 3
#define E820_NVS 4
#define E820_UNUSABLE 5

struct E820Entry {
  UINT64 Addr;
  UINT64 Size;
  UINT32 Type;
} __attribute__((packed));

struct E820Map {
  UINT32 EntryCount;
  struct E820Entry Map[E820_MAX];
};

#endif
