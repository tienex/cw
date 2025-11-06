/** @file
  KESU 4-ring protection implementation.

  This file implements the VMS-style 4-ring protection model with
  per-ring endianness, stack direction, and page table configuration.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "../../include/MmixCore.h"
#include "../../include/MmixTypes.h"
#include <string.h>

/**
  Transition to a different protection ring.

  Performs a ring transition with stack switching and context saving.
  Validates the transition is allowed (can only move to more privileged
  rings via call gates, and to less privileged rings via return).

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      NewRing       Target ring (0-3).
  @param[in]      IsCall        TRUE if ring transition via call, FALSE if return.

  @retval MMIX_SUCCESS          Ring transition completed successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Transition not allowed.

**/
MMIX_STATUS
MmixCpuTransitionRing (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           NewRing,
  IN     BOOLEAN         IsCall
  )
{
  if (CpuState == NULL || NewRing >= MMIX_RING_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // KESU extension must be enabled
  //
  if (!CpuState->KesuExtensionEnabled) {
    return MMIX_ERROR_UNSUPPORTED;
  }

  UINT8  CurrentRing = (UINT8)CpuState->PrivilegeLevel;

  //
  // Validate ring transition:
  // - Can only transition to MORE privileged ring (lower number) via call
  // - Can only transition to LESS privileged ring (higher number) via return
  //
  if (IsCall && NewRing >= CurrentRing) {
    return MMIX_ERROR_ACCESS_DENIED;
  }

  if (!IsCall && NewRing <= CurrentRing) {
    return MMIX_ERROR_ACCESS_DENIED;
  }

  //
  // Target ring must be enabled
  //
  if (!CpuState->RingConfig[NewRing].Enabled) {
    return MMIX_ERROR_ACCESS_DENIED;
  }

  //
  // Save current stack pointer to current ring config
  //
  UINT64  StackReg;
  MmixCpuReadRegister (CpuState, 254, &StackReg);  // $254 is typical stack pointer
  CpuState->RingConfig[CurrentRing].StackPointer = StackReg;

  //
  // Switch to new ring
  //
  CpuState->PrivilegeLevel = (MMIX_PRIVILEGE_LEVEL)NewRing;

  //
  // Load stack pointer from new ring config
  //
  MmixCpuWriteRegister (CpuState, 254, CpuState->RingConfig[NewRing].StackPointer);

  return MMIX_SUCCESS;
}

/**
  Get the current endianness mode.

  Returns the endianness mode for the current privilege ring.
  If KESU extension is disabled, returns big-endian (MMIX default).

  @param[in]      CpuState      Pointer to CPU state.

  @return  Current endianness mode.

**/
MMIX_ENDIAN_MODE
MmixCpuGetEndianness (
  IN MMIX_CPU_STATE  *CpuState
  )
{
  if (CpuState == NULL) {
    return MmixEndianBig;
  }

  //
  // If KESU extension is disabled, use big-endian (MMIX default)
  //
  if (!CpuState->KesuExtensionEnabled) {
    return MmixEndianBig;
  }

  //
  // Hypervisor mode uses ring 0 configuration
  //
  UINT8  Ring = (UINT8)CpuState->PrivilegeLevel;
  if (Ring >= MMIX_RING_COUNT) {
    Ring = 0;
  }

  return CpuState->RingConfig[Ring].EndiannessMode;
}

/**
  Get the current stack direction.

  Returns the stack growth direction for the current privilege ring.

  @param[in]      CpuState      Pointer to CPU state.

  @return  Current stack direction.

**/
MMIX_STACK_DIRECTION
MmixCpuGetStackDirection (
  IN MMIX_CPU_STATE  *CpuState
  )
{
  if (CpuState == NULL) {
    return MmixStackGrowsDown;
  }

  //
  // If KESU extension is disabled, use downward (common default)
  //
  if (!CpuState->KesuExtensionEnabled) {
    return MmixStackGrowsDown;
  }

  //
  // Hypervisor mode uses ring 0 configuration
  //
  UINT8  Ring = (UINT8)CpuState->PrivilegeLevel;
  if (Ring >= MMIX_RING_COUNT) {
    Ring = 0;
  }

  return CpuState->RingConfig[Ring].StackDirection;
}

/**
  Get the page table base for the current ring.

  Returns the page table base physical address for the current
  privilege level. Used by memory management for address translation.

  @param[in]      CpuState      Pointer to CPU state.

  @return  Page table base physical address.

**/
UINT64
MmixCpuGetPageTableBase (
  IN MMIX_CPU_STATE  *CpuState
  )
{
  if (CpuState == NULL) {
    return 0;
  }

  //
  // If KESU extension is disabled, use rPT (special register 37)
  //
  if (!CpuState->KesuExtensionEnabled) {
    UINT64  PageTableBase;
    MmixCpuReadSpecialRegister (CpuState, MMIX_rPT, &PageTableBase);
    return PageTableBase;
  }

  //
  // Hypervisor mode uses ring 0 configuration
  //
  UINT8  Ring = (UINT8)CpuState->PrivilegeLevel;
  if (Ring >= MMIX_RING_COUNT) {
    Ring = 0;
  }

  return CpuState->RingConfig[Ring].PageTableBase;
}

/**
  Configure a protection ring.

  Sets the endianness, stack direction, and page table base for
  a specific protection ring. Requires kernel privilege.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Ring          Ring number (0-3).
  @param[in]      Endianness    Endianness mode for this ring.
  @param[in]      StackDir      Stack growth direction.
  @param[in]      PageTableBase Page table base physical address.

  @retval MMIX_SUCCESS          Ring configured successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Insufficient privilege.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid ring number.

**/
MMIX_STATUS
MmixCpuConfigureRing (
  IN OUT MMIX_CPU_STATE       *CpuState,
  IN     UINT8                Ring,
  IN     MMIX_ENDIAN_MODE     Endianness,
  IN     MMIX_STACK_DIRECTION StackDir,
  IN     UINT64               PageTableBase
  )
{
  if (CpuState == NULL || Ring >= MMIX_RING_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // KESU extension must be enabled
  //
  if (!CpuState->KesuExtensionEnabled) {
    return MMIX_ERROR_UNSUPPORTED;
  }

  //
  // Must be in kernel mode (ring 0) to configure rings
  //
  if (CpuState->PrivilegeLevel != MmixPrivilegeKernel) {
    return MMIX_ERROR_ACCESS_DENIED;
  }

  //
  // Update ring configuration
  //
  CpuState->RingConfig[Ring].EndiannessMode = Endianness;
  CpuState->RingConfig[Ring].StackDirection = StackDir;
  CpuState->RingConfig[Ring].PageTableBase  = PageTableBase;
  CpuState->RingConfig[Ring].Enabled        = TRUE;

  return MMIX_SUCCESS;
}
