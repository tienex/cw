/** @file
  MMIX Device emulation interfaces.

  This file defines the common device interface and structures for all
  emulated hardware devices including interrupt controllers, framebuffer,
  network, serial, and input devices.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_DEVICES_H__
#define __MMIX_DEVICES_H__

#include "../MmixTypes.h"

//
// Device types
//

typedef enum {
  MmixDeviceTypeInterruptController = 0,
  MmixDeviceTypeFramebuffer = 1,
  MmixDeviceTypeKeyboard = 2,
  MmixDeviceTypeMouse = 3,
  MmixDeviceTypeNetwork = 4,
  MmixDeviceTypeSerial = 5,
  MmixDeviceTypeTimer = 6,
  MmixDeviceTypePci = 7
} MMIX_DEVICE_TYPE;

//
// PCI configuration space
//

#define PCI_CONFIG_VENDOR_ID      0x00
#define PCI_CONFIG_DEVICE_ID      0x02
#define PCI_CONFIG_COMMAND        0x04
#define PCI_CONFIG_STATUS         0x06
#define PCI_CONFIG_REVISION       0x08
#define PCI_CONFIG_CLASS_CODE     0x09
#define PCI_CONFIG_HEADER_TYPE    0x0E
#define PCI_CONFIG_BAR0           0x10
#define PCI_CONFIG_BAR1           0x14
#define PCI_CONFIG_BAR2           0x18
#define PCI_CONFIG_BAR3           0x1C
#define PCI_CONFIG_BAR4           0x20
#define PCI_CONFIG_BAR5           0x24
#define PCI_CONFIG_INTERRUPT_LINE 0x3C
#define PCI_CONFIG_INTERRUPT_PIN  0x3D

/**
  PCI device configuration.
**/
typedef struct {
  UINT16  VendorId;       ///< PCI vendor ID
  UINT16  DeviceId;       ///< PCI device ID
  UINT16  Command;        ///< Command register
  UINT16  Status;         ///< Status register
  UINT8   RevisionId;     ///< Revision ID
  UINT8   ClassCode[3];   ///< Class code
  UINT8   HeaderType;     ///< Header type
  UINT32  Bar[6];         ///< Base address registers
  UINT8   InterruptLine;  ///< Interrupt line
  UINT8   InterruptPin;   ///< Interrupt pin
  UINT64  BarSize[6];     ///< Size of each BAR region
  BOOLEAN BarIsMmio[6];   ///< TRUE if BAR is MMIO, FALSE if I/O
} PCI_DEVICE_CONFIG;

//
// Device state structure
//

/**
  Generic device state.

  Base structure for all device types. Specific devices extend this
  with their own state.
**/
struct _MMIX_DEVICE_STATE {
  ///
  /// Device type
  ///
  MMIX_DEVICE_TYPE  DeviceType;

  ///
  /// Device name
  ///
  CONST CHAR8       *Name;

  ///
  /// PCI configuration (if PCI device)
  ///
  PCI_DEVICE_CONFIG *PciConfig;

  ///
  /// I/O handler function
  ///
  MMIX_DEVICE_IO_HANDLER  IoHandler;

  ///
  /// Device-specific state
  ///
  VOID              *PrivateState;

  ///
  /// Pointer to emulator context
  ///
  MMIX_EMULATOR_CONTEXT  *EmulatorContext;
};

//
// Device functions
//

/**
  Initialize a device.

  @param[out]  DeviceState       Pointer to receive device state.
  @param[in]   DeviceType        Type of device to create.
  @param[in]   EmulatorContext   Emulator context.

  @retval MMIX_SUCCESS           Device initialized successfully.
  @retval Others                 Error occurred.

**/
MMIX_STATUS
MmixDeviceInitialize (
  OUT MMIX_DEVICE_STATE     **DeviceState,
  IN  MMIX_DEVICE_TYPE      DeviceType,
  IN  MMIX_EMULATOR_CONTEXT *EmulatorContext
  );

/**
  Perform I/O operation on device.

  @param[in]      DeviceState   Pointer to device state.
  @param[in]      Offset        Register offset.
  @param[in,out]  Value         Value to read/write.
  @param[in]      Size          Access size in bytes.
  @param[in]      Write         TRUE for write, FALSE for read.

  @retval MMIX_SUCCESS          Operation completed successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDeviceIo (
  IN     MMIX_DEVICE_STATE  *DeviceState,
  IN     UINT64             Offset,
  IN OUT VOID               *Value,
  IN     UINT32             Size,
  IN     BOOLEAN            Write
  );

/**
  Destroy a device.

  @param[in]  DeviceState       Device state to destroy.

**/
VOID
MmixDeviceDestroy (
  IN MMIX_DEVICE_STATE  *DeviceState
  );

#endif // __MMIX_DEVICES_H__
