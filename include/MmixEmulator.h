/** @file
  MMIX Emulator main interface.

  This is the primary public interface for the MMIX emulator. Applications
  should include only this header to use the emulator functionality.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_EMULATOR_H__
#define __MMIX_EMULATOR_H__

#include "MmixTypes.h"
#include "MmixCore.h"
#include "MmixMemory.h"
#include "devices/MmixDevices.h"

//
// Emulator configuration
//

/**
  Emulator configuration structure.

  Specifies initial configuration parameters for the emulator including
  memory size, device types, and feature enables.
**/
typedef struct {
  ///
  /// Physical memory size in bytes
  ///
  UINT64    MemorySize;

  ///
  /// Initial program counter value
  ///
  UINT64    InitialPc;

  ///
  /// Vector length in bytes (for SVE)
  ///
  UINT32    VectorLength;

  ///
  /// Enable hypervisor extensions
  ///
  BOOLEAN   EnableHypervisor;

  ///
  /// Enable vector extensions (SVE)
  ///
  BOOLEAN   EnableVectorExtensions;

  ///
  /// Enable matrix extensions (SME)
  ///
  BOOLEAN   EnableMatrixExtensions;

  ///
  /// Enable compressed instructions
  ///
  BOOLEAN   EnableCompressedInstructions;

  ///
  /// Enable framebuffer device
  ///
  BOOLEAN   EnableFramebuffer;

  ///
  /// Framebuffer width
  ///
  UINT32    FramebufferWidth;

  ///
  /// Framebuffer height
  ///
  UINT32    FramebufferHeight;

  ///
  /// Enable network device
  ///
  BOOLEAN   EnableNetwork;

  ///
  /// Enable serial ports
  ///
  BOOLEAN   EnableSerial;

  ///
  /// Number of serial ports (0-4)
  ///
  UINT32    SerialPortCount;
} MMIX_EMULATOR_CONFIG;

//
// Emulator context
//

/**
  Complete emulator state.

  Contains all state for a running MMIX emulator instance including
  CPU, memory, devices, and hypervisor state.
**/
struct _MMIX_EMULATOR_CONTEXT {
  ///
  /// CPU state
  ///
  MMIX_CPU_STATE    *CpuState;

  ///
  /// Memory state
  ///
  MMIX_MEMORY_STATE *MemoryState;

  ///
  /// Device array
  ///
  MMIX_DEVICE_STATE **Devices;

  ///
  /// Number of devices
  ///
  UINT32            DeviceCount;

  ///
  /// Configuration
  ///
  MMIX_EMULATOR_CONFIG  Config;

  ///
  /// Emulator is running
  ///
  BOOLEAN           Running;

  ///
  /// Exit requested
  ///
  BOOLEAN           ExitRequested;

  ///
  /// Exit code
  ///
  UINT64            ExitCode;
};

//
// Emulator functions
//

/**
  Create and initialize a new emulator instance.

  Allocates and initializes all emulator components including CPU,
  memory, and devices according to the configuration.

  @param[out]  Context           Pointer to receive emulator context.
  @param[in]   Config            Pointer to configuration structure.

  @retval MMIX_SUCCESS           Emulator created successfully.
  @retval MMIX_ERROR_OUT_OF_MEMORY  Failed to allocate memory.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid configuration.

**/
MMIX_STATUS
MmixEmulatorCreate (
  OUT MMIX_EMULATOR_CONTEXT  **Context,
  IN  MMIX_EMULATOR_CONFIG   *Config
  );

/**
  Run the emulator.

  Begins execution at the configured initial PC. Continues until
  an error occurs, the emulator is halted, or the exit is requested.

  @param[in,out]  Context       Emulator context.

  @retval MMIX_SUCCESS          Emulator exited normally.
  @retval Others                Error occurred during execution.

**/
MMIX_STATUS
MmixEmulatorRun (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context
  );

/**
  Single-step the emulator.

  Executes exactly one instruction and returns.

  @param[in,out]  Context       Emulator context.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixEmulatorStep (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context
  );

/**
  Reset the emulator.

  Resets CPU and device state to initial values. Memory contents
  are preserved.

  @param[in,out]  Context       Emulator context.

  @retval MMIX_SUCCESS          Emulator reset successfully.

**/
MMIX_STATUS
MmixEmulatorReset (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context
  );

/**
  Load a binary image into memory.

  Loads a binary file into emulator memory at the specified address.
  Useful for loading bootloaders or kernels.

  @param[in,out]  Context       Emulator context.
  @param[in]      ImagePath     Path to binary image file.
  @param[in]      LoadAddress   Physical address to load at.

  @retval MMIX_SUCCESS          Image loaded successfully.
  @retval MMIX_ERROR_NOT_FOUND  Image file not found.
  @retval Others                Error occurred during load.

**/
MMIX_STATUS
MmixEmulatorLoadImage (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *ImagePath,
  IN     UINT64                 LoadAddress
  );

/**
  Request emulator exit.

  Signals the emulator to exit at the next safe point with the
  specified exit code.

  @param[in,out]  Context       Emulator context.
  @param[in]      ExitCode      Exit code value.

**/
VOID
MmixEmulatorRequestExit (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     UINT64                 ExitCode
  );

/**
  Destroy an emulator instance.

  Frees all resources associated with the emulator including CPU,
  memory, and devices.

  @param[in]  Context           Emulator context to destroy.

**/
VOID
MmixEmulatorDestroy (
  IN MMIX_EMULATOR_CONTEXT  *Context
  );

//
// Utility functions
//

/**
  Get default emulator configuration.

  Returns a configuration structure with reasonable default values.

  @param[out]  Config           Pointer to receive default configuration.

**/
VOID
MmixEmulatorGetDefaultConfig (
  OUT MMIX_EMULATOR_CONFIG  *Config
  );

/**
  Raise an interrupt.

  Raises an interrupt on the emulated CPU. The interrupt will be
  delivered at the next opportunity based on interrupt masking.

  @param[in,out]  Context       Emulator context.
  @param[in]      InterruptNum  Interrupt number (0-63).

  @retval MMIX_SUCCESS          Interrupt raised successfully.

**/
MMIX_STATUS
MmixEmulatorRaiseInterrupt (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     UINT8                  InterruptNum
  );

/**
  Dump emulator state.

  Dumps CPU registers, memory, and device state to a file or stdout
  for debugging purposes.

  @param[in]  Context           Emulator context.
  @param[in]  OutputPath        Path to output file (NULL for stdout).

  @retval MMIX_SUCCESS          State dumped successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixEmulatorDumpState (
  IN MMIX_EMULATOR_CONTEXT  *Context,
  IN CONST CHAR8            *OutputPath OPTIONAL
  );

#endif // __MMIX_EMULATOR_H__
