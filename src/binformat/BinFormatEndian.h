/** @file
  Binary Format Endianness Utilities.

  Common byte-swapping and endianness detection utilities used by all
  binary format libraries (ELF, COFF, Mach-O, a.out, etc.).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __BINFORMAT_ENDIAN_H__
#define __BINFORMAT_ENDIAN_H__

#include "../../include/MmixTypes.h"

//
// Define INLINE for byte-swapping helper functions
//
#ifndef INLINE
#define INLINE static inline
#endif

/**
  Swap bytes in 16-bit value.

  @param[in]  Value             16-bit value to swap.

  @return Byte-swapped value.
**/
INLINE
UINT16
BinFormatSwap16 (
  IN  UINT16  Value
  )
{
  return ((Value & 0xFF) << 8) | ((Value >> 8) & 0xFF);
}

/**
  Swap bytes in 32-bit value.

  @param[in]  Value             32-bit value to swap.

  @return Byte-swapped value.
**/
INLINE
UINT32
BinFormatSwap32 (
  IN  UINT32  Value
  )
{
  return ((Value & 0x000000FF) << 24) |
         ((Value & 0x0000FF00) << 8) |
         ((Value & 0x00FF0000) >> 8) |
         ((Value & 0xFF000000) >> 24);
}

/**
  Swap bytes in 64-bit value.

  @param[in]  Value             64-bit value to swap.

  @return Byte-swapped value.
**/
INLINE
UINT64
BinFormatSwap64 (
  IN  UINT64  Value
  )
{
  return ((Value & 0x00000000000000FFULL) << 56) |
         ((Value & 0x000000000000FF00ULL) << 40) |
         ((Value & 0x0000000000FF0000ULL) << 24) |
         ((Value & 0x00000000FF000000ULL) << 8) |
         ((Value & 0x000000FF00000000ULL) >> 8) |
         ((Value & 0x0000FF0000000000ULL) >> 24) |
         ((Value & 0x00FF000000000000ULL) >> 40) |
         ((Value & 0xFF00000000000000ULL) >> 56);
}

/**
  Detect if host is little-endian.

  @return TRUE if host is little-endian, FALSE if big-endian.
**/
INLINE
BOOLEAN
BinFormatIsLittleEndianHost (
  VOID
  )
{
  UINT16 Test = 0x0001;
  return *((UINT8*)&Test) == 0x01;
}

/**
  Conditionally swap 16-bit value based on endianness flag.

  @param[in]  Value             Value to swap.
  @param[in]  NeedsByteSwap     TRUE if byte swapping is needed.

  @return Byte-swapped value if needed, original value otherwise.
**/
INLINE
UINT16
BinFormatConditionalSwap16 (
  IN  UINT16   Value,
  IN  BOOLEAN  NeedsByteSwap
  )
{
  return NeedsByteSwap ? BinFormatSwap16(Value) : Value;
}

/**
  Conditionally swap 32-bit value based on endianness flag.

  @param[in]  Value             Value to swap.
  @param[in]  NeedsByteSwap     TRUE if byte swapping is needed.

  @return Byte-swapped value if needed, original value otherwise.
**/
INLINE
UINT32
BinFormatConditionalSwap32 (
  IN  UINT32   Value,
  IN  BOOLEAN  NeedsByteSwap
  )
{
  return NeedsByteSwap ? BinFormatSwap32(Value) : Value;
}

/**
  Conditionally swap 64-bit value based on endianness flag.

  @param[in]  Value             Value to swap.
  @param[in]  NeedsByteSwap     TRUE if byte swapping is needed.

  @return Byte-swapped value if needed, original value otherwise.
**/
INLINE
UINT64
BinFormatConditionalSwap64 (
  IN  UINT64   Value,
  IN  BOOLEAN  NeedsByteSwap
  )
{
  return NeedsByteSwap ? BinFormatSwap64(Value) : Value;
}

#endif // __BINFORMAT_ENDIAN_H__
