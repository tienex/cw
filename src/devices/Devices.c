/** @file
  MMIX Device emulation stub implementation.

  This file provides stub implementations for device emulation.
  Full device implementations would be added in separate files.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include "../../include/devices/MmixDevices.h"

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
  )
{
  MMIX_DEVICE_STATE  *Device;

  if (DeviceState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate device state
  //
  Device = (MMIX_DEVICE_STATE *)malloc (sizeof (MMIX_DEVICE_STATE));
  if (Device == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  memset (Device, 0, sizeof (MMIX_DEVICE_STATE));

  Device->DeviceType = DeviceType;
  Device->EmulatorContext = EmulatorContext;
  Device->IoHandler = NULL;
  Device->PrivateState = NULL;
  Device->PciConfig = NULL;

  //
  // Set device name based on type
  //
  switch (DeviceType) {
    case MmixDeviceTypeInterruptController:
      Device->Name = "Apple AIC";
      break;
    case MmixDeviceTypeFramebuffer:
      Device->Name = "Framebuffer";
      break;
    case MmixDeviceTypeKeyboard:
      Device->Name = "ADB Keyboard";
      break;
    case MmixDeviceTypeMouse:
      Device->Name = "ADB Mouse";
      break;
    case MmixDeviceTypeNetwork:
      Device->Name = "VMXNet3";
      break;
    case MmixDeviceTypeSerial:
      Device->Name = "Zilog Serial";
      break;
    default:
      Device->Name = "Unknown Device";
      break;
  }

  *DeviceState = Device;
  return MMIX_SUCCESS;
}

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
  )
{
  if (DeviceState == NULL || Value == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Call device-specific handler if present
  //
  if (DeviceState->IoHandler != NULL) {
    return DeviceState->IoHandler (DeviceState, Offset, Value, Size, Write);
  }

  return MMIX_ERROR_UNSUPPORTED;
}

/**
  Destroy a device.

  @param[in]  DeviceState       Device state to destroy.

**/
VOID
MmixDeviceDestroy (
  IN MMIX_DEVICE_STATE  *DeviceState
  )
{
  if (DeviceState != NULL) {
    if (DeviceState->PrivateState != NULL) {
      free (DeviceState->PrivateState);
    }
    if (DeviceState->PciConfig != NULL) {
      free (DeviceState->PciConfig);
    }
    free (DeviceState);
  }
}
