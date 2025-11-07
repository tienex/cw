/** @file
  ELF Core Dump Reading and CONTEXT Conversion.

  Implementation of functions to convert between ELF NT_PRSTATUS notes
  and CONTEXT structures for debugging core dumps.

  Based on Linux kernel's elfcore.h and GDB's core file reading.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/binformat/ElfCore.h"

//
// x86-64 conversions
//

BINFORMAT_STATUS
ElfPrstatusToContextX64 (
  IN  CONST ELF_PRSTATUS_X86_64  *Prstatus,
  OUT AMD64_CONTEXT              *Context
  )
{
  if (Prstatus == NULL || Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the context
  //
  memset(Context, 0, sizeof(AMD64_CONTEXT));

  //
  // Set context flags
  //
  Context->ContextFlags = CONTEXT_AMD64_FULL | CONTEXT_AMD64_SEGMENTS;

  //
  // Copy general purpose registers
  //
  Context->Rax = Prstatus->pr_reg.rax;
  Context->Rbx = Prstatus->pr_reg.rbx;
  Context->Rcx = Prstatus->pr_reg.rcx;
  Context->Rdx = Prstatus->pr_reg.rdx;
  Context->Rsi = Prstatus->pr_reg.rsi;
  Context->Rdi = Prstatus->pr_reg.rdi;
  Context->Rbp = Prstatus->pr_reg.rbp;
  Context->Rsp = Prstatus->pr_reg.rsp;
  Context->R8  = Prstatus->pr_reg.r8;
  Context->R9  = Prstatus->pr_reg.r9;
  Context->R10 = Prstatus->pr_reg.r10;
  Context->R11 = Prstatus->pr_reg.r11;
  Context->R12 = Prstatus->pr_reg.r12;
  Context->R13 = Prstatus->pr_reg.r13;
  Context->R14 = Prstatus->pr_reg.r14;
  Context->R15 = Prstatus->pr_reg.r15;

  //
  // Copy instruction pointer and flags
  //
  Context->Rip = Prstatus->pr_reg.rip;
  Context->EFlags = (UINT32)Prstatus->pr_reg.eflags;

  //
  // Copy segment registers
  //
  Context->SegCs = (UINT16)Prstatus->pr_reg.cs;
  Context->SegSs = (UINT16)Prstatus->pr_reg.ss;
  Context->SegDs = (UINT16)Prstatus->pr_reg.ds;
  Context->SegEs = (UINT16)Prstatus->pr_reg.es;
  Context->SegFs = (UINT16)Prstatus->pr_reg.fs;
  Context->SegGs = (UINT16)Prstatus->pr_reg.gs;

  return BINFORMAT_SUCCESS;
}

BINFORMAT_STATUS
ElfContextToPrstatusX64 (
  IN  CONST AMD64_CONTEXT        *Context,
  OUT ELF_PRSTATUS_X86_64        *Prstatus
  )
{
  if (Context == NULL || Prstatus == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the prstatus structure
  //
  memset(Prstatus, 0, sizeof(ELF_PRSTATUS_X86_64));

  //
  // Copy general purpose registers
  //
  Prstatus->pr_reg.rax = Context->Rax;
  Prstatus->pr_reg.rbx = Context->Rbx;
  Prstatus->pr_reg.rcx = Context->Rcx;
  Prstatus->pr_reg.rdx = Context->Rdx;
  Prstatus->pr_reg.rsi = Context->Rsi;
  Prstatus->pr_reg.rdi = Context->Rdi;
  Prstatus->pr_reg.rbp = Context->Rbp;
  Prstatus->pr_reg.rsp = Context->Rsp;
  Prstatus->pr_reg.r8  = Context->R8;
  Prstatus->pr_reg.r9  = Context->R9;
  Prstatus->pr_reg.r10 = Context->R10;
  Prstatus->pr_reg.r11 = Context->R11;
  Prstatus->pr_reg.r12 = Context->R12;
  Prstatus->pr_reg.r13 = Context->R13;
  Prstatus->pr_reg.r14 = Context->R14;
  Prstatus->pr_reg.r15 = Context->R15;

  //
  // Copy instruction pointer and flags
  //
  Prstatus->pr_reg.rip = Context->Rip;
  Prstatus->pr_reg.eflags = Context->EFlags;

  //
  // Copy segment registers
  //
  Prstatus->pr_reg.cs = Context->SegCs;
  Prstatus->pr_reg.ss = Context->SegSs;
  Prstatus->pr_reg.ds = Context->SegDs;
  Prstatus->pr_reg.es = Context->SegEs;
  Prstatus->pr_reg.fs = Context->SegFs;
  Prstatus->pr_reg.gs = Context->SegGs;

  //
  // Set fpvalid if floating-point context is present
  //
  if ((Context->ContextFlags & CONTEXT_AMD64_FLOATING_POINT) != 0) {
    Prstatus->pr_fpvalid = 1;
  }

  return BINFORMAT_SUCCESS;
}

//
// i386 conversions
//

BINFORMAT_STATUS
ElfPrstatusToContextI386 (
  IN  CONST ELF_PRSTATUS_I386    *Prstatus,
  OUT I386_CONTEXT               *Context
  )
{
  if (Prstatus == NULL || Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the context
  //
  memset(Context, 0, sizeof(I386_CONTEXT));

  //
  // Set context flags
  //
  Context->ContextFlags = CONTEXT_I386_FULL;

  //
  // Copy general purpose registers
  //
  Context->Eax = Prstatus->pr_reg.eax;
  Context->Ebx = Prstatus->pr_reg.ebx;
  Context->Ecx = Prstatus->pr_reg.ecx;
  Context->Edx = Prstatus->pr_reg.edx;
  Context->Esi = Prstatus->pr_reg.esi;
  Context->Edi = Prstatus->pr_reg.edi;
  Context->Ebp = Prstatus->pr_reg.ebp;
  Context->Esp = Prstatus->pr_reg.esp;

  //
  // Copy instruction pointer and flags
  //
  Context->Eip = Prstatus->pr_reg.eip;
  Context->EFlags = Prstatus->pr_reg.eflags;

  //
  // Copy segment registers
  //
  Context->SegCs = (UINT32)Prstatus->pr_reg.xcs;
  Context->SegSs = (UINT32)Prstatus->pr_reg.xss;
  Context->SegDs = (UINT32)Prstatus->pr_reg.xds;
  Context->SegEs = (UINT32)Prstatus->pr_reg.xes;
  Context->SegFs = (UINT32)Prstatus->pr_reg.xfs;
  Context->SegGs = (UINT32)Prstatus->pr_reg.xgs;

  return BINFORMAT_SUCCESS;
}

BINFORMAT_STATUS
ElfContextToPrstatusI386 (
  IN  CONST I386_CONTEXT         *Context,
  OUT ELF_PRSTATUS_I386          *Prstatus
  )
{
  if (Context == NULL || Prstatus == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the prstatus structure
  //
  memset(Prstatus, 0, sizeof(ELF_PRSTATUS_I386));

  //
  // Copy general purpose registers
  //
  Prstatus->pr_reg.eax = Context->Eax;
  Prstatus->pr_reg.ebx = Context->Ebx;
  Prstatus->pr_reg.ecx = Context->Ecx;
  Prstatus->pr_reg.edx = Context->Edx;
  Prstatus->pr_reg.esi = Context->Esi;
  Prstatus->pr_reg.edi = Context->Edi;
  Prstatus->pr_reg.ebp = Context->Ebp;
  Prstatus->pr_reg.esp = Context->Esp;

  //
  // Copy instruction pointer and flags
  //
  Prstatus->pr_reg.eip = Context->Eip;
  Prstatus->pr_reg.eflags = Context->EFlags;

  //
  // Copy segment registers
  //
  Prstatus->pr_reg.xcs = Context->SegCs;
  Prstatus->pr_reg.xss = Context->SegSs;
  Prstatus->pr_reg.xds = Context->SegDs;
  Prstatus->pr_reg.xes = Context->SegEs;
  Prstatus->pr_reg.xfs = Context->SegFs;
  Prstatus->pr_reg.xgs = Context->SegGs;

  //
  // Set fpvalid if floating-point context is present
  //
  if ((Context->ContextFlags & CONTEXT_I386_FLOATING_POINT) != 0) {
    Prstatus->pr_fpvalid = 1;
  }

  return BINFORMAT_SUCCESS;
}

//
// ARM64 conversions
//

BINFORMAT_STATUS
ElfPrstatusToContextArm64 (
  IN  CONST ELF_PRSTATUS_ARM64   *Prstatus,
  OUT ARM64_NT_CONTEXT           *Context
  )
{
  UINT32  i;

  if (Prstatus == NULL || Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the context
  //
  memset(Context, 0, sizeof(ARM64_NT_CONTEXT));

  //
  // Set context flags
  //
  Context->ContextFlags = CONTEXT_ARM64_FULL;

  //
  // Copy general purpose registers X0-X30
  //
  Context->X0  = Prstatus->pr_reg.regs[0];
  Context->X1  = Prstatus->pr_reg.regs[1];
  Context->X2  = Prstatus->pr_reg.regs[2];
  Context->X3  = Prstatus->pr_reg.regs[3];
  Context->X4  = Prstatus->pr_reg.regs[4];
  Context->X5  = Prstatus->pr_reg.regs[5];
  Context->X6  = Prstatus->pr_reg.regs[6];
  Context->X7  = Prstatus->pr_reg.regs[7];
  Context->X8  = Prstatus->pr_reg.regs[8];
  Context->X9  = Prstatus->pr_reg.regs[9];
  Context->X10 = Prstatus->pr_reg.regs[10];
  Context->X11 = Prstatus->pr_reg.regs[11];
  Context->X12 = Prstatus->pr_reg.regs[12];
  Context->X13 = Prstatus->pr_reg.regs[13];
  Context->X14 = Prstatus->pr_reg.regs[14];
  Context->X15 = Prstatus->pr_reg.regs[15];
  Context->X16 = Prstatus->pr_reg.regs[16];
  Context->X17 = Prstatus->pr_reg.regs[17];
  Context->X18 = Prstatus->pr_reg.regs[18];
  Context->X19 = Prstatus->pr_reg.regs[19];
  Context->X20 = Prstatus->pr_reg.regs[20];
  Context->X21 = Prstatus->pr_reg.regs[21];
  Context->X22 = Prstatus->pr_reg.regs[22];
  Context->X23 = Prstatus->pr_reg.regs[23];
  Context->X24 = Prstatus->pr_reg.regs[24];
  Context->X25 = Prstatus->pr_reg.regs[25];
  Context->X26 = Prstatus->pr_reg.regs[26];
  Context->X27 = Prstatus->pr_reg.regs[27];
  Context->X28 = Prstatus->pr_reg.regs[28];
  Context->X29 = Prstatus->pr_reg.regs[29];  // Frame pointer
  Context->X30 = Prstatus->pr_reg.regs[30];  // Link register

  //
  // Copy stack pointer, program counter, and processor state
  //
  Context->Sp = Prstatus->pr_reg.sp;
  Context->Pc = Prstatus->pr_reg.pc;
  Context->Cpsr = (UINT32)Prstatus->pr_reg.pstate;

  return BINFORMAT_SUCCESS;
}

BINFORMAT_STATUS
ElfContextToPrstatusArm64 (
  IN  CONST ARM64_NT_CONTEXT     *Context,
  OUT ELF_PRSTATUS_ARM64         *Prstatus
  )
{
  UINT32  i;

  if (Context == NULL || Prstatus == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the prstatus structure
  //
  memset(Prstatus, 0, sizeof(ELF_PRSTATUS_ARM64));

  //
  // Copy general purpose registers X0-X30
  //
  Prstatus->pr_reg.regs[0]  = Context->X0;
  Prstatus->pr_reg.regs[1]  = Context->X1;
  Prstatus->pr_reg.regs[2]  = Context->X2;
  Prstatus->pr_reg.regs[3]  = Context->X3;
  Prstatus->pr_reg.regs[4]  = Context->X4;
  Prstatus->pr_reg.regs[5]  = Context->X5;
  Prstatus->pr_reg.regs[6]  = Context->X6;
  Prstatus->pr_reg.regs[7]  = Context->X7;
  Prstatus->pr_reg.regs[8]  = Context->X8;
  Prstatus->pr_reg.regs[9]  = Context->X9;
  Prstatus->pr_reg.regs[10] = Context->X10;
  Prstatus->pr_reg.regs[11] = Context->X11;
  Prstatus->pr_reg.regs[12] = Context->X12;
  Prstatus->pr_reg.regs[13] = Context->X13;
  Prstatus->pr_reg.regs[14] = Context->X14;
  Prstatus->pr_reg.regs[15] = Context->X15;
  Prstatus->pr_reg.regs[16] = Context->X16;
  Prstatus->pr_reg.regs[17] = Context->X17;
  Prstatus->pr_reg.regs[18] = Context->X18;
  Prstatus->pr_reg.regs[19] = Context->X19;
  Prstatus->pr_reg.regs[20] = Context->X20;
  Prstatus->pr_reg.regs[21] = Context->X21;
  Prstatus->pr_reg.regs[22] = Context->X22;
  Prstatus->pr_reg.regs[23] = Context->X23;
  Prstatus->pr_reg.regs[24] = Context->X24;
  Prstatus->pr_reg.regs[25] = Context->X25;
  Prstatus->pr_reg.regs[26] = Context->X26;
  Prstatus->pr_reg.regs[27] = Context->X27;
  Prstatus->pr_reg.regs[28] = Context->X28;
  Prstatus->pr_reg.regs[29] = Context->X29;
  Prstatus->pr_reg.regs[30] = Context->X30;

  //
  // Copy stack pointer, program counter, and processor state
  //
  Prstatus->pr_reg.sp = Context->Sp;
  Prstatus->pr_reg.pc = Context->Pc;
  Prstatus->pr_reg.pstate = Context->Cpsr;

  //
  // Set fpvalid if floating-point context is present
  //
  if ((Context->ContextFlags & CONTEXT_ARM64_FLOATING_POINT) != 0) {
    Prstatus->pr_fpvalid = 1;
  }

  return BINFORMAT_SUCCESS;
}

//
// ARM 32-bit conversions
//

BINFORMAT_STATUS
ElfPrstatusToContextArm (
  IN  CONST ELF_PRSTATUS_ARM     *Prstatus,
  OUT ARM_CONTEXT                *Context
  )
{
  if (Prstatus == NULL || Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the context
  //
  memset(Context, 0, sizeof(ARM_CONTEXT));

  //
  // Set context flags
  //
  Context->ContextFlags = CONTEXT_ARM_FULL;

  //
  // Copy general purpose registers R0-R12
  //
  Context->R0  = Prstatus->pr_reg.regs[0];
  Context->R1  = Prstatus->pr_reg.regs[1];
  Context->R2  = Prstatus->pr_reg.regs[2];
  Context->R3  = Prstatus->pr_reg.regs[3];
  Context->R4  = Prstatus->pr_reg.regs[4];
  Context->R5  = Prstatus->pr_reg.regs[5];
  Context->R6  = Prstatus->pr_reg.regs[6];
  Context->R7  = Prstatus->pr_reg.regs[7];
  Context->R8  = Prstatus->pr_reg.regs[8];
  Context->R9  = Prstatus->pr_reg.regs[9];
  Context->R10 = Prstatus->pr_reg.regs[10];
  Context->R11 = Prstatus->pr_reg.regs[11];
  Context->R12 = Prstatus->pr_reg.regs[12];

  //
  // Copy special registers (R13-R15 and CPSR)
  //
  Context->Sp   = Prstatus->pr_reg.regs[13];  // R13 = SP
  Context->Lr   = Prstatus->pr_reg.regs[14];  // R14 = LR
  Context->Pc   = Prstatus->pr_reg.regs[15];  // R15 = PC
  Context->Cpsr = Prstatus->pr_reg.regs[16];  // CPSR

  return BINFORMAT_SUCCESS;
}

BINFORMAT_STATUS
ElfContextToPrstatusArm (
  IN  CONST ARM_CONTEXT          *Context,
  OUT ELF_PRSTATUS_ARM           *Prstatus
  )
{
  if (Context == NULL || Prstatus == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Clear the prstatus structure
  //
  memset(Prstatus, 0, sizeof(ELF_PRSTATUS_ARM));

  //
  // Copy general purpose registers R0-R12
  //
  Prstatus->pr_reg.regs[0]  = Context->R0;
  Prstatus->pr_reg.regs[1]  = Context->R1;
  Prstatus->pr_reg.regs[2]  = Context->R2;
  Prstatus->pr_reg.regs[3]  = Context->R3;
  Prstatus->pr_reg.regs[4]  = Context->R4;
  Prstatus->pr_reg.regs[5]  = Context->R5;
  Prstatus->pr_reg.regs[6]  = Context->R6;
  Prstatus->pr_reg.regs[7]  = Context->R7;
  Prstatus->pr_reg.regs[8]  = Context->R8;
  Prstatus->pr_reg.regs[9]  = Context->R9;
  Prstatus->pr_reg.regs[10] = Context->R10;
  Prstatus->pr_reg.regs[11] = Context->R11;
  Prstatus->pr_reg.regs[12] = Context->R12;

  //
  // Copy special registers
  //
  Prstatus->pr_reg.regs[13] = Context->Sp;
  Prstatus->pr_reg.regs[14] = Context->Lr;
  Prstatus->pr_reg.regs[15] = Context->Pc;
  Prstatus->pr_reg.regs[16] = Context->Cpsr;
  Prstatus->pr_reg.regs[17] = 0;  // orig_r0 - not present in Context

  //
  // Set fpvalid if floating-point context is present
  //
  if ((Context->ContextFlags & CONTEXT_ARM_FLOATING_POINT) != 0) {
    Prstatus->pr_fpvalid = 1;
  }

  return BINFORMAT_SUCCESS;
}

//
// Core file reading implementation
//

// ELF note header structure
typedef struct {
  UINT32  n_namesz;   // Name size
  UINT32  n_descsz;   // Descriptor size
  UINT32  n_type;     // Note type
} ELF_NOTE_HEADER;

/**
  Read ELF core file and extract thread contexts.
**/
BINFORMAT_STATUS
ElfReadCoreFile (
  IN  CONST CHAR8         *FilePath,
  OUT UINT32              *ThreadCount,
  OUT ELF_CORE_THREAD     **Threads
  )
{
  FILE              *File;
  UINT8             Ident[16];
  BOOLEAN           Is64Bit;
  BOOLEAN           IsLittleEndian;
  UINT16            Machine;
  UINT16            PhNum;
  UINT64            PhOff;
  UINT32            ThreadCapacity;
  UINT32            Count;
  ELF_CORE_THREAD   *ThreadArray;
  BINFORMAT_STATUS  Status;

  if (FilePath == NULL || ThreadCount == NULL || Threads == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  *ThreadCount = 0;
  *Threads = NULL;

  //
  // Open the core file
  //
  File = fopen(FilePath, "rb");
  if (File == NULL) {
    return BINFORMAT_ERROR_IO;
  }

  //
  // Read ELF identification
  //
  if (fread(Ident, 1, 16, File) != 16) {
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  //
  // Verify ELF magic
  //
  if (Ident[0] != 0x7F || Ident[1] != 'E' ||
      Ident[2] != 'L' || Ident[3] != 'F') {
    fclose(File);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  //
  // Get file class (32 or 64-bit)
  //
  Is64Bit = (Ident[4] == 2);
  IsLittleEndian = (Ident[5] == 1);

  //
  // Read ELF header to get program header info
  //
  fseek(File, 0, SEEK_SET);

  if (Is64Bit) {
    //
    // ELF64 header
    //
    UINT8 Header[64];
    if (fread(Header, 1, 64, File) != 64) {
      fclose(File);
      return BINFORMAT_ERROR_IO;
    }

    // Extract machine type (e_machine at offset 18)
    Machine = *(UINT16 *)(Header + 18);

    // Extract program header info
    PhOff = *(UINT64 *)(Header + 32);  // e_phoff
    PhNum = *(UINT16 *)(Header + 56);  // e_phnum
  } else {
    //
    // ELF32 header
    //
    UINT8 Header[52];
    if (fread(Header, 1, 52, File) != 52) {
      fclose(File);
      return BINFORMAT_ERROR_IO;
    }

    // Extract machine type (e_machine at offset 18)
    Machine = *(UINT16 *)(Header + 18);

    // Extract program header info
    PhOff = *(UINT32 *)(Header + 28);  // e_phoff
    PhNum = *(UINT16 *)(Header + 44);  // e_phnum
  }

  //
  // Allocate thread array (start with capacity of 16)
  //
  ThreadCapacity = 16;
  ThreadArray = (ELF_CORE_THREAD *)malloc(ThreadCapacity * sizeof(ELF_CORE_THREAD));
  if (ThreadArray == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }
  Count = 0;

  //
  // Scan program headers for PT_NOTE segments
  //
  for (UINT32 i = 0; i < PhNum; i++) {
    UINT64  NoteOffset;
    UINT64  NoteSize;
    UINT8   *NoteData;
    UINT8   *NotePtr;
    UINT8   *NoteEnd;

    fseek(File, PhOff + (i * (Is64Bit ? 56 : 32)), SEEK_SET);

    if (Is64Bit) {
      UINT8 PhEntry[56];
      if (fread(PhEntry, 1, 56, File) != 56) {
        continue;
      }

      UINT32 Type = *(UINT32 *)(PhEntry);
      if (Type != 4) {  // PT_NOTE = 4
        continue;
      }

      NoteOffset = *(UINT64 *)(PhEntry + 8);   // p_offset
      NoteSize = *(UINT64 *)(PhEntry + 32);    // p_filesz
    } else {
      UINT8 PhEntry[32];
      if (fread(PhEntry, 1, 32, File) != 32) {
        continue;
      }

      UINT32 Type = *(UINT32 *)(PhEntry);
      if (Type != 4) {  // PT_NOTE = 4
        continue;
      }

      NoteOffset = *(UINT32 *)(PhEntry + 4);   // p_offset
      NoteSize = *(UINT32 *)(PhEntry + 16);    // p_filesz
    }

    //
    // Read the note data
    //
    NoteData = (UINT8 *)malloc(NoteSize);
    if (NoteData == NULL) {
      continue;
    }

    fseek(File, NoteOffset, SEEK_SET);
    if (fread(NoteData, 1, NoteSize, File) != NoteSize) {
      free(NoteData);
      continue;
    }

    //
    // Parse notes in this segment
    //
    NotePtr = NoteData;
    NoteEnd = NoteData + NoteSize;

    while (NotePtr + sizeof(ELF_NOTE_HEADER) <= NoteEnd) {
      ELF_NOTE_HEADER *NoteHdr = (ELF_NOTE_HEADER *)NotePtr;
      UINT32  NameSize = NoteHdr->n_namesz;
      UINT32  DescSize = NoteHdr->n_descsz;
      UINT32  NoteType = NoteHdr->n_type;
      UINT8   *Name;
      UINT8   *Desc;

      // Align sizes to 4-byte boundaries
      UINT32  NameSizeAligned = (NameSize + 3) & ~3;
      UINT32  DescSizeAligned = (DescSize + 3) & ~3;

      Name = NotePtr + sizeof(ELF_NOTE_HEADER);
      Desc = Name + NameSizeAligned;

      // Check if we have enough data
      if (Desc + DescSize > NoteEnd) {
        break;
      }

      //
      // Check if this is an NT_PRSTATUS note (type 1)
      //
      if (NoteType == NT_PRSTATUS && DescSize > 0) {
        //
        // Expand array if needed
        //
        if (Count >= ThreadCapacity) {
          ThreadCapacity *= 2;
          ELF_CORE_THREAD *NewArray = (ELF_CORE_THREAD *)realloc(
            ThreadArray,
            ThreadCapacity * sizeof(ELF_CORE_THREAD)
          );
          if (NewArray == NULL) {
            free(NoteData);
            goto cleanup;
          }
          ThreadArray = NewArray;
        }

        //
        // Convert based on architecture
        //
        VOID *Context = NULL;
        UINT32 ContextSize = 0;
        INT32 ThreadId = 0;
        INT32 SignalNum = 0;

        if (Machine == 62) {  // EM_X86_64
          if (DescSize >= sizeof(ELF_PRSTATUS_X86_64)) {
            ELF_PRSTATUS_X86_64 *Prstatus = (ELF_PRSTATUS_X86_64 *)Desc;
            AMD64_CONTEXT *Ctx = (AMD64_CONTEXT *)malloc(sizeof(AMD64_CONTEXT));
            if (Ctx != NULL) {
              if (ElfPrstatusToContextX64(Prstatus, Ctx) == BINFORMAT_SUCCESS) {
                Context = Ctx;
                ContextSize = sizeof(AMD64_CONTEXT);
                ThreadId = Prstatus->common.pr_pid;
                SignalNum = Prstatus->common.pr_cursig;
              } else {
                free(Ctx);
              }
            }
          }
        } else if (Machine == 3) {  // EM_386
          if (DescSize >= sizeof(ELF_PRSTATUS_I386)) {
            ELF_PRSTATUS_I386 *Prstatus = (ELF_PRSTATUS_I386 *)Desc;
            I386_CONTEXT *Ctx = (I386_CONTEXT *)malloc(sizeof(I386_CONTEXT));
            if (Ctx != NULL) {
              if (ElfPrstatusToContextI386(Prstatus, Ctx) == BINFORMAT_SUCCESS) {
                Context = Ctx;
                ContextSize = sizeof(I386_CONTEXT);
                ThreadId = Prstatus->common.pr_pid;
                SignalNum = Prstatus->common.pr_cursig;
              } else {
                free(Ctx);
              }
            }
          }
        } else if (Machine == 183) {  // EM_AARCH64
          if (DescSize >= sizeof(ELF_PRSTATUS_ARM64)) {
            ELF_PRSTATUS_ARM64 *Prstatus = (ELF_PRSTATUS_ARM64 *)Desc;
            ARM64_NT_CONTEXT *Ctx = (ARM64_NT_CONTEXT *)malloc(sizeof(ARM64_NT_CONTEXT));
            if (Ctx != NULL) {
              if (ElfPrstatusToContextArm64(Prstatus, Ctx) == BINFORMAT_SUCCESS) {
                Context = Ctx;
                ContextSize = sizeof(ARM64_NT_CONTEXT);
                ThreadId = Prstatus->common.pr_pid;
                SignalNum = Prstatus->common.pr_cursig;
              } else {
                free(Ctx);
              }
            }
          }
        } else if (Machine == 40) {  // EM_ARM
          if (DescSize >= sizeof(ELF_PRSTATUS_ARM)) {
            ELF_PRSTATUS_ARM *Prstatus = (ELF_PRSTATUS_ARM *)Desc;
            ARM_CONTEXT *Ctx = (ARM_CONTEXT *)malloc(sizeof(ARM_CONTEXT));
            if (Ctx != NULL) {
              if (ElfPrstatusToContextArm(Prstatus, Ctx) == BINFORMAT_SUCCESS) {
                Context = Ctx;
                ContextSize = sizeof(ARM_CONTEXT);
                ThreadId = Prstatus->common.pr_pid;
                SignalNum = Prstatus->common.pr_cursig;
              } else {
                free(Ctx);
              }
            }
          }
        }

        if (Context != NULL) {
          ThreadArray[Count].ThreadId = ThreadId;
          ThreadArray[Count].SignalNumber = SignalNum;
          ThreadArray[Count].Context = Context;
          ThreadArray[Count].ContextSize = ContextSize;
          Count++;
        }
      }

      // Move to next note
      NotePtr = Desc + DescSizeAligned;
    }

    free(NoteData);
  }

  fclose(File);

  if (Count == 0) {
    free(ThreadArray);
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  *ThreadCount = Count;
  *Threads = ThreadArray;
  return BINFORMAT_SUCCESS;

cleanup:
  //
  // Cleanup on error
  //
  for (UINT32 i = 0; i < Count; i++) {
    if (ThreadArray[i].Context != NULL) {
      free(ThreadArray[i].Context);
    }
  }
  free(ThreadArray);
  fclose(File);
  return BINFORMAT_ERROR_OUT_OF_MEMORY;
}

/**
  Free resources allocated by ElfReadCoreFile.
**/
VOID
ElfFreeCoreThreads (
  IN  ELF_CORE_THREAD     *Threads,
  IN  UINT32              ThreadCount
  )
{
  if (Threads == NULL) {
    return;
  }

  for (UINT32 i = 0; i < ThreadCount; i++) {
    if (Threads[i].Context != NULL) {
      free(Threads[i].Context);
    }
  }

  free(Threads);
}
