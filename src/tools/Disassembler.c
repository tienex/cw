/** @file
  MMIX Disassembler implementation.

  This file implements the disassembler for converting MMIX binary
  instructions into human-readable assembly code.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <string.h>
#include "../../include/MmixDisasm.h"

//
// Opcode mnemonic table for standard instructions
//
STATIC CONST CHAR8  *mOpcodeNames[256] = {
  // 0x00-0x0F: Load instructions
  "LDB",   "LDBU",  "LDW",   "LDWU",  "LDT",   "LDTU",  "LDO",   "LDOU",
  "LDSF",  "LDHT",  "LDUNC", "LDVTS", "PRELD", "PREGO", "GO",    "SWYM",
  // 0x10-0x1F: Store instructions
  "STB",   "STBU",  "STW",   "STWU",  "STT",   "STTU",  "STO",   "STOU",
  "STSF",  "STHT",  "STCO",  "STUNC", "SYNCD", "PREST", "SYNCID", "PUSHGO",
  // 0x20-0x2F: Arithmetic
  "ADD",   "ADDI",  "ADDU",  "ADDUI", "SUB",   "SUBI",  "SUBU",  "SUBUI",
  "MUL",   "MULI",  "MULU",  "MULUI", "DIV",   "DIVI",  "DIVU",  "DIVUI",
  // 0x30-0x3F: Shift/Compare
  "2ADDU", "4ADDU", "8ADDU", "16ADDU", "SL",   "SLI",   "SLU",   "SLUI",
  "SR",    "SRI",   "SRU",   "SRUI",  "CMP",   "CMPI",  "CMPU",  "CMPUI",
  // 0x40-0x4F: Logical
  "AND",   "ANDI",  "OR",    "ORI",   "XOR",   "XORI",  "ANDN",  "ANDNI",
  "ORN",   "ORNI",  "NAND",  "NANDI", "NOR",   "NORI",  "NXOR",  "NXORI",
  // 0x50-0x5F: Branches
  "BN",    "BNB",   "BZ",    "BZB",   "BP",    "BPB",   "BOD",   "BODB",
  "BNN",   "BNNB",  "BNZ",   "BNZB",  "BNP",   "BNPB",  "BEV",   "BEVB",
  // 0x60-0x6F: Floating-point
  "FADD",  "FSUB",  "FMUL",  "FDIV",  "FREM",  "FSQRT", "FINT",  "FIX",
  "FIXU",  "FLOT",  "FLOTU", "SFLOT", "SFLOTU", "FCMP", "FEQL",  "FUN",
  // 0x70-0x7F: More FP/Special
  "FCMPE", "FEQLE", "FUNE",  "FADD8", "FSUB8", "FMUL8", "FMA",   "FMS",
  "FNMA",  "FNMS",  "FREM8", "FRECIP", "FRSQRTE", "LDDP", "STDP", "TRAP",
  // 0x80-0x8F: Vector operations (standard encoding)
  "VADD",  "VSUB",  "VMUL",  "VDIV",  "VMIN",  "VMAX",  "VAND",  "VOR",
  "VXOR",  "VNOT",  "VSHL",  "VSHR",  "VSHRU", "VCMP",  "VCMPU", "VSEL",
  // 0x90-0x9F: More Vector
  "VLD",   "VST",   "VLDFF", "VLDNF", "VSTNT", "VGATHER", "VSCATTER", "VPREFETCH",
  "VFADD", "VFSUB", "VFMUL", "VFDIV", "VFSQRT", "VFMA", "VFMIN", "VFMAX",
  // 0xA0-0xAF: Matrix operations
  "MMMUL", "MMAD",  "MMTRANS", "MMZERO", "MMLD", "MMST", "MMCFG", "MMSTART",
  "MMCONV", "MMPOOL", "MMACT", "MMBATCH", "MMQUANT", "MMDEQUANT", "MMPACK", "MMUNPACK",
  // 0xB0-0xBF: ML/AI operations
  "MLDOT", "MLGEMM", "MLCONV2D", "MLPOOL2D", "MLRELU", "MLSIGMOID", "MLTANH", "MLSOFTMAX",
  "MLQUANT8", "MLDEQUANT8", "MLBF16", "MLFP16", "MLFP8E4M3", "MLFP8E5M2", "MLTENSOR", "MLREDUCE",
  // 0xC0-0xCF: Special/System
  "GET",   "GETA",  "PUT",   "PUTI",  "POP",   "RESUME", "SAVE",  "UNSAVE",
  "SYNC",  "SWYM1", "LDVL",  "STVL",  "RDVL",  "SETVL", "PUSHJ", "PUSHJB",
  // 0xD0-0xDF: More system
  "CSR",   "CSRI",  "CSRR",  "CSRW",  "CSRS",  "CSRC",  "FENCE", "FENCEI",
  "MFENCE", "SFENCE", "LFENCE", "INVTLB", "INVLPG", "INVPCID", "WBINVD", "CLFLUSH",
  // 0xE0-0xEF: Hypervisor
  "HVLOAD", "HVSTORE", "HVGET", "HVPUT", "HVSAVE", "HVRESUME", "HVTRAP", "HVRET",
  "VMRUN", "VMEXIT", "VMLOAD", "VMSAVE", "VMCALL", "VMFUNC", "VMGEXIT", "VMXON",
  // 0xF0-0xFF: Reserved/Extended
  "EXT0",  "EXT1",  "EXT2",  "EXT3",  "EXT4",  "EXT5",  "EXT6",  "EXT7",
  "EXT8",  "EXT9",  "EXTA",  "EXTB",  "EXTC",  "EXTD",  "EXTE",  "EXTF"
};

//
// Compressed opcode mnemonic table (top 4 bits 0x8-0xF)
//
STATIC CONST CHAR8  *mCompressedOpcodeNames[128] = {
  // 0x80-0x87: Compressed arithmetic
  "C.ADD",  "C.SUB",  "C.MUL",  "C.AND",  "C.OR",   "C.XOR",  "C.SL",   "C.SR",
  // 0x88-0x8F: Compressed immediate
  "C.ADDI", "C.SUBI", "C.ANDI", "C.ORI",  "C.XORI", "C.SLI",  "C.SRI",  "C.LI",
  // 0x90-0x97: Compressed load/store
  "C.LDB",  "C.LDW",  "C.LDT",  "C.LDO",  "C.STB",  "C.STW",  "C.STT",  "C.STO",
  // 0x98-0x9F: Compressed branches
  "C.BZ",   "C.BNZ",  "C.BP",   "C.BN",   "C.J",    "C.JR",   "C.JAL",  "C.JALR",
  // 0xA0-0xA7: Compressed special
  "C.MV",   "C.NOT",  "C.NEG",  "C.SEXT", "C.ZEXT", "C.LUI",  "C.NOP",  "C.SWYM",
  // 0xA8-0xAF: Reserved compressed
  "C.RES8", "C.RES9", "C.RESA", "C.RESB", "C.RESC", "C.RESD", "C.RESE", "C.RESF",
  // 0xB0-0xB7: Compressed FP
  "C.FADD", "C.FSUB", "C.FMUL", "C.FDIV", "C.FLD",  "C.FST",  "C.FCMP", "C.FMOV",
  // 0xB8-0xBF: Compressed vector
  "C.VADD", "C.VSUB", "C.VMUL", "C.VLD",  "C.VST",  "C.VDOT", "C.VMOV", "C.VCFG",
  // 0xC0-0xC7: Compressed system
  "C.ECALL", "C.EBREAK", "C.URET", "C.SRET", "C.MRET", "C.WFI", "C.FENCE", "C.CSR",
  // 0xC8-0xCF: Reserved
  "C.RES18", "C.RES19", "C.RES1A", "C.RES1B", "C.RES1C", "C.RES1D", "C.RES1E", "C.RES1F",
  // 0xD0-0xDF: Reserved
  "C.RES20", "C.RES21", "C.RES22", "C.RES23", "C.RES24", "C.RES25", "C.RES26", "C.RES27",
  "C.RES28", "C.RES29", "C.RES2A", "C.RES2B", "C.RES2C", "C.RES2D", "C.RES2E", "C.RES2F",
  // 0xE0-0xEF: Reserved
  "C.RES30", "C.RES31", "C.RES32", "C.RES33", "C.RES34", "C.RES35", "C.RES36", "C.RES37",
  "C.RES38", "C.RES39", "C.RES3A", "C.RES3B", "C.RES3C", "C.RES3D", "C.RES3E", "C.RES3F",
  // 0xF0-0xFF: Reserved
  "C.RES40", "C.RES41", "C.RES42", "C.RES43", "C.RES44", "C.RES45", "C.RES46", "C.RES47",
  "C.RES48", "C.RES49", "C.RES4A", "C.RES4B", "C.RES4C", "C.RES4D", "C.RES4E", "C.RES4F"
};

/**
  Format a register name.

  @param[out]     Buffer        Buffer to receive register name.
  @param[in]      BufferSize    Size of buffer.
  @param[in]      RegNum        Register number.
  @param[in]      IsSpecial     TRUE for special registers.

**/
STATIC
VOID
FormatRegister (
  OUT CHAR8    *Buffer,
  IN  UINT32   BufferSize,
  IN  UINT8    RegNum,
  IN  BOOLEAN  IsSpecial
  )
{
  if (IsSpecial) {
    snprintf (Buffer, BufferSize, "rS%u", RegNum);
  } else {
    snprintf (Buffer, BufferSize, "$%u", RegNum);
  }
}

/**
  Disassemble a standard (32-bit) instruction.

  @param[in]      Instruction   The instruction to disassemble.
  @param[in]      Address       The instruction address.
  @param[out]     Buffer        Output buffer.
  @param[in]      BufferSize    Buffer size.

  @retval MMIX_SUCCESS          Success.

**/
STATIC
MMIX_STATUS
DisassembleStandard (
  IN  UINT32  Instruction,
  IN  UINT64  Address,
  OUT CHAR8   *Buffer,
  IN  UINT32  BufferSize
  )
{
  UINT8        Opcode;
  UINT8        X;
  UINT8        Y;
  UINT8        Z;
  UINT16       YZ;
  CONST CHAR8  *Mnemonic;
  CHAR8        XReg[16];
  CHAR8        YReg[16];
  CHAR8        ZReg[16];

  //
  // Extract fields
  //
  Opcode = (Instruction >> 24) & 0xFF;
  X = (Instruction >> 16) & 0xFF;
  Y = (Instruction >> 8) & 0xFF;
  Z = Instruction & 0xFF;
  YZ = Instruction & 0xFFFF;

  Mnemonic = mOpcodeNames[Opcode];

  //
  // Format registers
  //
  FormatRegister (XReg, sizeof (XReg), X, FALSE);
  FormatRegister (YReg, sizeof (YReg), Y, FALSE);
  FormatRegister (ZReg, sizeof (ZReg), Z, FALSE);

  //
  // Format based on instruction type
  //
  if (Opcode >= 0x00 && Opcode <= 0x0D) {
    // Load instructions: LDB $X,$Y,$Z
    snprintf (Buffer, BufferSize, "%-8s %s,%s,%s", Mnemonic, XReg, YReg, ZReg);
  } else if (Opcode >= 0x10 && Opcode <= 0x1D) {
    // Store instructions: STB $X,$Y,$Z
    snprintf (Buffer, BufferSize, "%-8s %s,%s,%s", Mnemonic, XReg, YReg, ZReg);
  } else if ((Opcode & 0x01) == 0x01) {
    // Immediate instructions: ADDI $X,$Y,Z
    snprintf (Buffer, BufferSize, "%-8s %s,%s,%u", Mnemonic, XReg, YReg, Z);
  } else if (Opcode >= 0x50 && Opcode <= 0x5F) {
    // Branch instructions: BZ $X,offset
    INT64   Offset = (INT16)YZ;
    UINT64  Target = Address + (Offset * 4);
    snprintf (Buffer, BufferSize, "%-8s %s,0x%llx", Mnemonic, XReg, (unsigned long long)Target);
  } else if (Opcode == 0xC1) {
    // GETA: GETA $X,YZ
    INT64   Offset = (INT16)YZ;
    UINT64  Target = Address + (Offset * 4);
    snprintf (Buffer, BufferSize, "%-8s %s,0x%llx", Mnemonic, XReg, (unsigned long long)Target);
  } else if (Opcode == 0xC0 || Opcode == 0xC2) {
    // GET/PUT: Special register operations
    CHAR8  SpecReg[16];
    FormatRegister (SpecReg, sizeof (SpecReg), Z, TRUE);
    if (Opcode == 0xC0) {
      // GET $X,rSZ
      snprintf (Buffer, BufferSize, "%-8s %s,%s", Mnemonic, XReg, SpecReg);
    } else {
      // PUT rSX,$Y
      FormatRegister (SpecReg, sizeof (SpecReg), X, TRUE);
      snprintf (Buffer, BufferSize, "%-8s %s,%s", Mnemonic, SpecReg, YReg);
    }
  } else {
    // Standard three-register format: ADD $X,$Y,$Z
    snprintf (Buffer, BufferSize, "%-8s %s,%s,%s", Mnemonic, XReg, YReg, ZReg);
  }

  return MMIX_SUCCESS;
}

/**
  Disassemble a compressed (16-bit) instruction.

  @param[in]      Instruction   The instruction (in lower 16 bits).
  @param[in]      Address       The instruction address.
  @param[out]     Buffer        Output buffer.
  @param[in]      BufferSize    Buffer size.

  @retval MMIX_SUCCESS          Success.

**/
STATIC
MMIX_STATUS
DisassembleCompressed (
  IN  UINT32  Instruction,
  IN  UINT64  Address,
  OUT CHAR8   *Buffer,
  IN  UINT32  BufferSize
  )
{
  UINT16       Inst16;
  UINT8        Opcode;
  UINT8        Rd;
  UINT8        Rs1;
  UINT8        Rs2;
  UINT8        Imm;
  CONST CHAR8  *Mnemonic;
  CHAR8        RdReg[16];
  CHAR8        Rs1Reg[16];
  CHAR8        Rs2Reg[16];

  Inst16 = (UINT16)(Instruction & 0xFFFF);

  //
  // Extract compressed instruction fields
  // Format: [15:12] opcode, [11:8] rd/rs1, [7:4] rs2/imm, [3:0] imm
  //
  Opcode = (Inst16 >> 12) & 0xF;
  Rd = (Inst16 >> 8) & 0xF;
  Rs1 = Rd;  // Often rd and rs1 are same in compressed
  Rs2 = (Inst16 >> 4) & 0xF;
  Imm = Inst16 & 0xF;

  //
  // Map to extended opcode space
  //
  UINT8  ExtOpcode = ((Opcode & 0x7) << 4) | ((Inst16 >> 8) & 0xF);
  if (ExtOpcode >= 128) {
    ExtOpcode = 0;
  }
  Mnemonic = mCompressedOpcodeNames[ExtOpcode];

  //
  // Format registers
  //
  FormatRegister (RdReg, sizeof (RdReg), Rd, FALSE);
  FormatRegister (Rs1Reg, sizeof (Rs1Reg), Rs1, FALSE);
  FormatRegister (Rs2Reg, sizeof (Rs2Reg), Rs2, FALSE);

  //
  // Format based on compressed type
  //
  if ((Inst16 >> 13) == 0x4) {
    // Jump instruction: C.J offset
    INT16   Offset = (INT16)((Inst16 & 0x1FFF) << 3) >> 3;  // Sign extend
    UINT64  Target = Address + (Offset * 2);
    snprintf (Buffer, BufferSize, "%-8s 0x%llx", Mnemonic, (unsigned long long)Target);
  } else if ((Inst16 >> 12) == 0x8 || (Inst16 >> 12) == 0x9) {
    // Arithmetic with immediate: C.ADDI $rd,$rs1,imm
    INT8  SignedImm = (INT8)((Inst16 & 0xFF) << 4) >> 4;  // Sign extend
    snprintf (Buffer, BufferSize, "%-8s %s,%d", Mnemonic, RdReg, SignedImm);
  } else if ((Inst16 >> 12) == 0xA || (Inst16 >> 12) == 0xB) {
    // Load/Store: C.LDO $rd,$rs1,offset
    UINT8  Offset = ((Inst16 >> 4) & 0xF) << 2;
    snprintf (Buffer, BufferSize, "%-8s %s,%s,%u", Mnemonic, RdReg, Rs1Reg, Offset);
  } else {
    // Register-register: C.ADD $rd,$rs1,$rs2
    snprintf (Buffer, BufferSize, "%-8s %s,%s,%s", Mnemonic, RdReg, Rs1Reg, Rs2Reg);
  }

  return MMIX_SUCCESS;
}

/**
  Disassemble a single MMIX instruction.

  @param[in]      Instruction   The 32-bit instruction to disassemble.
  @param[in]      Address       The address of the instruction.
  @param[out]     Buffer        Buffer to receive the assembly text.
  @param[in]      BufferSize    Size of the buffer in bytes.

  @retval MMIX_SUCCESS          Instruction disassembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDisassemble (
  IN  UINT32  Instruction,
  IN  UINT64  Address,
  OUT CHAR8   *Buffer,
  IN  UINT32  BufferSize
  )
{
  UINT8  TopNibble;

  if (Buffer == NULL || BufferSize == 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Check if compressed instruction
  //
  TopNibble = (Instruction >> 28) & 0xF;
  if (TopNibble >= 0x8) {
    return DisassembleCompressed (Instruction, Address, Buffer, BufferSize);
  } else {
    return DisassembleStandard (Instruction, Address, Buffer, BufferSize);
  }
}

/**
  Disassemble a memory region.

  @param[in]      Memory        Pointer to memory region.
  @param[in]      Size          Size of memory region in bytes.
  @param[in]      StartAddress  Virtual address of first instruction.
  @param[out]     Buffer        Buffer to receive the assembly text.
  @param[in]      BufferSize    Size of the buffer in bytes.

  @retval MMIX_SUCCESS          Region disassembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDisassembleRegion (
  IN  CONST UINT8  *Memory,
  IN  UINT64       Size,
  IN  UINT64       StartAddress,
  OUT CHAR8        *Buffer,
  IN  UINT32       BufferSize
  )
{
  UINT64       Offset;
  UINT32       Instruction;
  CHAR8        LineBuffer[128];
  MMIX_STATUS  Status;
  UINT32       InstSize;
  UINT32       Used;

  if (Memory == NULL || Buffer == NULL || BufferSize == 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Buffer[0] = '\0';
  Used = 0;
  Offset = 0;

  while (Offset < Size) {
    //
    // Read instruction (assume big-endian)
    //
    if (Offset + 4 > Size) {
      break;
    }

    Instruction = (Memory[Offset] << 24) |
                  (Memory[Offset + 1] << 16) |
                  (Memory[Offset + 2] << 8) |
                  Memory[Offset + 3];

    //
    // Disassemble
    //
    Status = MmixDisassemble (Instruction, StartAddress + Offset, LineBuffer, sizeof (LineBuffer));
    if (Status != MMIX_SUCCESS) {
      break;
    }

    //
    // Determine instruction size
    //
    InstSize = MmixGetInstructionSize (Instruction);

    //
    // Append to output buffer
    //
    INT32  Written = snprintf (
                       Buffer + Used,
                       BufferSize - Used,
                       "  %016llx:  %s\n",
                       (unsigned long long)(StartAddress + Offset),
                       LineBuffer
                       );

    if (Written < 0 || (UINT32)Written >= BufferSize - Used) {
      return MMIX_ERROR_BUFFER_TOO_SMALL;
    }

    Used += Written;
    Offset += InstSize;
  }

  return MMIX_SUCCESS;
}

/**
  Get the size of an instruction at a given address.

  @param[in]      Instruction   The instruction word.

  @return  2 for compressed instructions, 4 for standard instructions.

**/
UINT32
MmixGetInstructionSize (
  IN  UINT32  Instruction
  )
{
  UINT8  TopNibble = (Instruction >> 28) & 0xF;
  return (TopNibble >= 0x8) ? 2 : 4;
}
