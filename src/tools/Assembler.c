/** @file
  MMIX Assembler implementation.

  This file implements the assembler for converting MMIX assembly
  source code into binary object files.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/MmixAsm.h"

//
// Opcode table for instruction encoding
//
typedef struct {
  CONST CHAR8  *Mnemonic;
  UINT8        Opcode;
  BOOLEAN      HasImmediate;
} OPCODE_ENTRY;

STATIC CONST OPCODE_ENTRY  mOpcodeTable[] = {
  // Load instructions
  { "LDB",   0x00, FALSE }, { "LDBU",  0x01, FALSE }, { "LDW",   0x02, FALSE }, { "LDWU",  0x03, FALSE },
  { "LDT",   0x04, FALSE }, { "LDTU",  0x05, FALSE }, { "LDO",   0x06, FALSE }, { "LDOU",  0x07, FALSE },
  // Store instructions
  { "STB",   0x10, FALSE }, { "STBU",  0x11, FALSE }, { "STW",   0x12, FALSE }, { "STWU",  0x13, FALSE },
  { "STT",   0x14, FALSE }, { "STTU",  0x15, FALSE }, { "STO",   0x16, FALSE }, { "STOU",  0x17, FALSE },
  // Arithmetic
  { "ADD",   0x20, FALSE }, { "ADDI",  0x21, TRUE  }, { "ADDU",  0x22, FALSE }, { "ADDUI", 0x23, TRUE  },
  { "SUB",   0x24, FALSE }, { "SUBI",  0x25, TRUE  }, { "SUBU",  0x26, FALSE }, { "SUBUI", 0x27, TRUE  },
  { "MUL",   0x28, FALSE }, { "MULI",  0x29, TRUE  }, { "MULU",  0x2A, FALSE }, { "MULUI", 0x2B, TRUE  },
  { "DIV",   0x2C, FALSE }, { "DIVI",  0x2D, TRUE  }, { "DIVU",  0x2E, FALSE }, { "DIVUI", 0x2F, TRUE  },
  // Shift/Compare
  { "SL",    0x34, FALSE }, { "SLI",   0x35, TRUE  }, { "SLU",   0x36, FALSE }, { "SLUI",  0x37, TRUE  },
  { "SR",    0x38, FALSE }, { "SRI",   0x39, TRUE  }, { "SRU",   0x3A, FALSE }, { "SRUI",  0x3B, TRUE  },
  { "CMP",   0x3C, FALSE }, { "CMPI",  0x3D, TRUE  }, { "CMPU",  0x3E, FALSE }, { "CMPUI", 0x3F, TRUE  },
  // Logical
  { "AND",   0x40, FALSE }, { "ANDI",  0x41, TRUE  }, { "OR",    0x42, FALSE }, { "ORI",   0x43, TRUE  },
  { "XOR",   0x44, FALSE }, { "XORI",  0x45, TRUE  }, { "ANDN",  0x46, FALSE }, { "ANDNI", 0x47, TRUE  },
  { "ORN",   0x48, FALSE }, { "ORNI",  0x49, TRUE  }, { "NAND",  0x4A, FALSE }, { "NANDI", 0x4B, TRUE  },
  { "NOR",   0x4C, FALSE }, { "NORI",  0x4D, TRUE  }, { "NXOR",  0x4E, FALSE }, { "NXORI", 0x4F, TRUE  },
  // Branches
  { "BN",    0x50, FALSE }, { "BNB",   0x51, FALSE }, { "BZ",    0x52, FALSE }, { "BZB",   0x53, FALSE },
  { "BP",    0x54, FALSE }, { "BPB",   0x55, FALSE }, { "BOD",   0x56, FALSE }, { "BODB",  0x57, FALSE },
  { "BNN",   0x58, FALSE }, { "BNNB",  0x59, FALSE }, { "BNZ",   0x5A, FALSE }, { "BNZB",  0x5B, FALSE },
  { "BNP",   0x5C, FALSE }, { "BNPB",  0x5D, FALSE }, { "BEV",   0x5E, FALSE }, { "BEVB",  0x5F, FALSE },
  // Floating-point
  { "FADD",  0x60, FALSE }, { "FSUB",  0x61, FALSE }, { "FMUL",  0x62, FALSE }, { "FDIV",  0x63, FALSE },
  { "FSQRT", 0x65, FALSE }, { "FINT",  0x66, FALSE }, { "FIX",   0x67, FALSE }, { "FIXU",  0x68, FALSE },
  { "FLOT",  0x69, FALSE }, { "FLOTU", 0x6A, FALSE }, { "FCMP",  0x6D, FALSE },
  // Special
  { "GET",   0xC0, FALSE }, { "GETA",  0xC1, FALSE }, { "PUT",   0xC2, FALSE }, { "PUTI",  0xC3, TRUE  },
  { "TRAP",  0x7F, FALSE },
  // Compressed instructions
  { "C.ADD", 0x80, FALSE }, { "C.SUB", 0x81, FALSE }, { "C.MUL", 0x82, FALSE },
  { "C.ADDI",0x88, TRUE  }, { "C.LI",  0x8F, TRUE  },
  { "C.LDO", 0x93, FALSE }, { "C.STO", 0x97, FALSE },
  { "C.J",   0x9C, FALSE }, { "C.JAL", 0x9E, FALSE },
  { NULL,    0x00, FALSE }
};

/**
  Find opcode entry by mnemonic.

  @param[in]      Mnemonic      The mnemonic string.

  @return  Pointer to opcode entry, or NULL if not found.

**/
STATIC
CONST OPCODE_ENTRY *
FindOpcode (
  IN  CONST CHAR8  *Mnemonic
  )
{
  for (UINT32 i = 0; mOpcodeTable[i].Mnemonic != NULL; i++) {
    if (strcasecmp (Mnemonic, mOpcodeTable[i].Mnemonic) == 0) {
      return &mOpcodeTable[i];
    }
  }
  return NULL;
}

/**
  Parse a register reference.

  @param[in]      Str           String to parse (e.g., "$1", "rS5").
  @param[out]     RegNum        Parsed register number.
  @param[out]     IsSpecial     TRUE if special register.

  @retval MMIX_SUCCESS          Parsed successfully.
  @retval Others                Parse error.

**/
STATIC
MMIX_STATUS
ParseRegister (
  IN  CONST CHAR8  *Str,
  OUT UINT8        *RegNum,
  OUT BOOLEAN      *IsSpecial
  )
{
  if (Str[0] == '$') {
    //
    // General register
    //
    *IsSpecial = FALSE;
    *RegNum = (UINT8)atoi (Str + 1);
    return MMIX_SUCCESS;
  } else if (strncasecmp (Str, "rS", 2) == 0) {
    //
    // Special register
    //
    *IsSpecial = TRUE;
    *RegNum = (UINT8)atoi (Str + 2);
    return MMIX_SUCCESS;
  }

  return MMIX_ERROR_INVALID_PARAMETER;
}

/**
  Parse an immediate value.

  @param[in]      Str           String to parse.
  @param[out]     Value         Parsed value.

  @retval MMIX_SUCCESS          Parsed successfully.
  @retval Others                Parse error.

**/
STATIC
MMIX_STATUS
ParseImmediate (
  IN  CONST CHAR8  *Str,
  OUT INT64        *Value
  )
{
  if (Str[0] == '0' && (Str[1] == 'x' || Str[1] == 'X')) {
    //
    // Hexadecimal
    //
    *Value = strtoll (Str, NULL, 16);
  } else if (Str[0] == '0' && (Str[1] == 'b' || Str[1] == 'B')) {
    //
    // Binary
    //
    *Value = strtoll (Str + 2, NULL, 2);
  } else {
    //
    // Decimal
    //
    *Value = strtoll (Str, NULL, 10);
  }

  return MMIX_SUCCESS;
}

/**
  Find or create a symbol.

  @param[in,out]  Context       Assembler context.
  @param[in]      Name          Symbol name.
  @param[out]     Index         Symbol index.

  @retval MMIX_SUCCESS          Symbol found or created.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
FindOrCreateSymbol (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     CONST CHAR8       *Name,
  OUT    UINT32            *Index
  )
{
  //
  // Search existing symbols
  //
  for (UINT32 i = 0; i < Context->SymbolCount; i++) {
    if (strcmp (Context->Symbols[i].Name, Name) == 0) {
      *Index = i;
      return MMIX_SUCCESS;
    }
  }

  //
  // Create new symbol
  //
  if (Context->SymbolCount >= MMIX_MAX_SYMBOLS) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  *Index = Context->SymbolCount++;
  strncpy (Context->Symbols[*Index].Name, Name, sizeof (Context->Symbols[*Index].Name) - 1);
  Context->Symbols[*Index].Value = 0;
  Context->Symbols[*Index].Size = 0;
  Context->Symbols[*Index].Type = MmixSymbolLocal;
  Context->Symbols[*Index].Bind = MmixBindLocal;
  Context->Symbols[*Index].Section = Context->CurrentSection;
  Context->Symbols[*Index].Defined = FALSE;

  return MMIX_SUCCESS;
}

/**
  Add relocation entry.

  @param[in,out]  Context       Assembler context.
  @param[in]      Type          Relocation type.
  @param[in]      Symbol        Symbol index.
  @param[in]      Offset        Offset in section.
  @param[in]      Addend        Addend value.

  @retval MMIX_SUCCESS          Relocation added.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
AddRelocation (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     MMIX_RELOC_TYPE   Type,
  IN     UINT32            Symbol,
  IN     UINT64            Offset,
  IN     INT64             Addend
  )
{
  if (Context->RelocationCount >= MMIX_MAX_RELOCATIONS) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  MMIX_RELOCATION  *Reloc = &Context->Relocations[Context->RelocationCount++];
  Reloc->Type = Type;
  Reloc->Symbol = Symbol;
  Reloc->Offset = Offset;
  Reloc->Addend = Addend;
  Reloc->Section = Context->CurrentSection;

  return MMIX_SUCCESS;
}

/**
  Emit bytes to current section.

  @param[in,out]  Context       Assembler context.
  @param[in]      Data          Data to emit.
  @param[in]      Size          Size in bytes.

  @retval MMIX_SUCCESS          Data emitted.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
EmitBytes (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     CONST UINT8       *Data,
  IN     UINT32            Size
  )
{
  MMIX_SECTION  *Section = &Context->Sections[Context->CurrentSection];

  //
  // Ensure capacity
  //
  if (Section->Size + Size > Section->DataCapacity) {
    UINT32  NewCapacity = (Section->DataCapacity == 0) ? 4096 : Section->DataCapacity * 2;
    while (NewCapacity < Section->Size + Size) {
      NewCapacity *= 2;
    }

    UINT8  *NewData = (UINT8 *)realloc (Section->Data, NewCapacity);
    if (NewData == NULL) {
      return MMIX_ERROR_OUT_OF_MEMORY;
    }

    Section->Data = NewData;
    Section->DataCapacity = NewCapacity;
  }

  //
  // Copy data
  //
  memcpy (Section->Data + Section->Size, Data, Size);
  Section->Size += Size;
  Context->CurrentAddress += Size;

  return MMIX_SUCCESS;
}

/**
  Emit a 32-bit instruction.

  @param[in,out]  Context       Assembler context.
  @param[in]      Instruction   The instruction word.

  @retval MMIX_SUCCESS          Instruction emitted.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
EmitInstruction (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     UINT32            Instruction
  )
{
  UINT8  Bytes[4];

  //
  // Big-endian encoding
  //
  Bytes[0] = (Instruction >> 24) & 0xFF;
  Bytes[1] = (Instruction >> 16) & 0xFF;
  Bytes[2] = (Instruction >> 8) & 0xFF;
  Bytes[3] = Instruction & 0xFF;

  return EmitBytes (Context, Bytes, 4);
}

/**
  Create a new assembler context.

  @param[out]     Context       Pointer to receive context.

  @retval MMIX_SUCCESS          Context created successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmCreate (
  OUT MMIX_ASM_CONTEXT  **Context
  )
{
  MMIX_ASM_CONTEXT  *Ctx;

  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Ctx = (MMIX_ASM_CONTEXT *)calloc (1, sizeof (MMIX_ASM_CONTEXT));
  if (Ctx == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  //
  // Create default .text section
  //
  Ctx->SectionCount = 1;
  strncpy (Ctx->Sections[0].Name, ".text", sizeof (Ctx->Sections[0].Name) - 1);
  Ctx->Sections[0].Type = MmixSectionText;
  Ctx->Sections[0].Alignment = 4;
  Ctx->CurrentSection = 0;
  Ctx->CurrentAddress = 0;

  *Context = Ctx;
  return MMIX_SUCCESS;
}

/**
  Destroy an assembler context.

  @param[in]      Context       Assembler context.

**/
VOID
MmixAsmDestroy (
  IN  MMIX_ASM_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return;
  }

  //
  // Free section data
  //
  for (UINT32 i = 0; i < Context->SectionCount; i++) {
    if (Context->Sections[i].Data != NULL) {
      free (Context->Sections[i].Data);
    }
  }

  free (Context);
}

/**
  Assemble a single line of assembly.

  @param[in,out]  Context       Assembler context.
  @param[in]      Line          Assembly line.

  @retval MMIX_SUCCESS          Line assembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmAssembleLine (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     CONST CHAR8       *Line
  )
{
  CHAR8   LineCopy[1024];
  CHAR8   *Token;
  CHAR8   *Saveptr;

  if (Context == NULL || Line == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Copy line for tokenization
  //
  strncpy (LineCopy, Line, sizeof (LineCopy) - 1);
  LineCopy[sizeof (LineCopy) - 1] = '\0';

  //
  // Remove comments
  //
  CHAR8  *Comment = strchr (LineCopy, '#');
  if (Comment != NULL) {
    *Comment = '\0';
  }

  Comment = strchr (LineCopy, ';');
  if (Comment != NULL) {
    *Comment = '\0';
  }

  //
  // Trim whitespace
  //
  CHAR8  *Start = LineCopy;
  while (isspace (*Start)) {
    Start++;
  }

  if (*Start == '\0') {
    return MMIX_SUCCESS;  // Empty line
  }

  //
  // Check for label
  //
  CHAR8  *Colon = strchr (Start, ':');
  if (Colon != NULL) {
    *Colon = '\0';
    UINT32  SymIndex;
    FindOrCreateSymbol (Context, Start, &SymIndex);
    Context->Symbols[SymIndex].Value = Context->CurrentAddress;
    Context->Symbols[SymIndex].Defined = TRUE;
    Start = Colon + 1;
    while (isspace (*Start)) {
      Start++;
    }
    if (*Start == '\0') {
      return MMIX_SUCCESS;
    }
  }

  //
  // Parse instruction
  //
  Token = strtok_r (Start, " ,\t", &Saveptr);
  if (Token == NULL) {
    return MMIX_SUCCESS;
  }

  //
  // Check for directive
  //
  if (Token[0] == '.') {
    if (strcasecmp (Token, ".text") == 0) {
      Context->CurrentSection = 0;
    } else if (strcasecmp (Token, ".data") == 0) {
      if (Context->SectionCount < 2) {
        Context->SectionCount = 2;
        strncpy (Context->Sections[1].Name, ".data", sizeof (Context->Sections[1].Name) - 1);
        Context->Sections[1].Type = MmixSectionData;
        Context->Sections[1].Alignment = 8;
      }
      Context->CurrentSection = 1;
    } else if (strcasecmp (Token, ".global") == 0 || strcasecmp (Token, ".globl") == 0) {
      Token = strtok_r (NULL, " ,\t", &Saveptr);
      if (Token != NULL) {
        UINT32  SymIndex;
        FindOrCreateSymbol (Context, Token, &SymIndex);
        Context->Symbols[SymIndex].Bind = MmixBindGlobal;
      }
    }
    return MMIX_SUCCESS;
  }

  //
  // Find opcode
  //
  CONST OPCODE_ENTRY  *Opcode = FindOpcode (Token);
  if (Opcode == NULL) {
    fprintf (stderr, "Unknown instruction: %s\n", Token);
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Parse operands
  //
  CHAR8  *Op1 = strtok_r (NULL, " ,\t", &Saveptr);
  CHAR8  *Op2 = strtok_r (NULL, " ,\t", &Saveptr);
  CHAR8  *Op3 = strtok_r (NULL, " ,\t", &Saveptr);

  if (Op1 == NULL) {
    fprintf (stderr, "Missing operands for: %s\n", Token);
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Parse registers
  //
  UINT8    X = 0, Y = 0, Z = 0;
  BOOLEAN  IsSpec;

  ParseRegister (Op1, &X, &IsSpec);

  if (Op2 != NULL) {
    if (Op2[0] == '$' || strncasecmp (Op2, "rS", 2) == 0) {
      ParseRegister (Op2, &Y, &IsSpec);
    } else {
      //
      // Immediate
      //
      INT64  Imm;
      ParseImmediate (Op2, &Imm);
      Y = (UINT8)(Imm & 0xFF);
    }
  }

  if (Op3 != NULL) {
    if (Op3[0] == '$' || strncasecmp (Op3, "rS", 2) == 0) {
      ParseRegister (Op3, &Z, &IsSpec);
    } else {
      //
      // Immediate or label
      //
      if (isdigit (Op3[0]) || Op3[0] == '-' || Op3[0] == '0') {
        INT64  Imm;
        ParseImmediate (Op3, &Imm);
        Z = (UINT8)(Imm & 0xFF);
      } else {
        //
        // Label reference - add relocation
        //
        UINT32  SymIndex;
        FindOrCreateSymbol (Context, Op3, &SymIndex);
        AddRelocation (Context, MmixRelocPcRel32, SymIndex, Context->CurrentAddress, 0);
        Z = 0;  // Will be filled by linker
      }
    }
  }

  //
  // Encode and emit instruction
  //
  UINT32  Instruction = ((UINT32)Opcode->Opcode << 24) | ((UINT32)X << 16) | ((UINT32)Y << 8) | Z;
  return EmitInstruction (Context, Instruction);
}

/**
  Assemble a source file.

  @param[in,out]  Context       Assembler context.
  @param[in]      FilePath      Path to source file.

  @retval MMIX_SUCCESS          File assembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmAssembleFile (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     CONST CHAR8       *FilePath
  )
{
  FILE          *File;
  CHAR8         Line[1024];
  MMIX_STATUS   Status;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  File = fopen (FilePath, "r");
  if (File == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  strncpy (Context->CurrentFile, FilePath, sizeof (Context->CurrentFile) - 1);
  Context->LineNumber = 0;

  while (fgets (Line, sizeof (Line), File) != NULL) {
    Context->LineNumber++;
    Status = MmixAsmAssembleLine (Context, Line);
    if (Status != MMIX_SUCCESS) {
      fprintf (stderr, "Error at %s:%u\n", FilePath, Context->LineNumber);
      fclose (File);
      return Status;
    }
  }

  fclose (File);
  return MMIX_SUCCESS;
}

/**
  Resolve all symbols and relocations.

  @param[in,out]  Context       Assembler context.

  @retval MMIX_SUCCESS          Resolution successful.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmResolve (
  IN OUT MMIX_ASM_CONTEXT  *Context
  )
{
  //
  // Check for undefined symbols
  //
  for (UINT32 i = 0; i < Context->SymbolCount; i++) {
    if (!Context->Symbols[i].Defined && Context->Symbols[i].Type != MmixSymbolExternal) {
      fprintf (stderr, "Undefined symbol: %s\n", Context->Symbols[i].Name);
      return MMIX_ERROR_NOT_FOUND;
    }
  }

  //
  // Apply relocations
  //
  for (UINT32 i = 0; i < Context->RelocationCount; i++) {
    MMIX_RELOCATION  *Reloc = &Context->Relocations[i];
    MMIX_SYMBOL      *Symbol = &Context->Symbols[Reloc->Symbol];
    MMIX_SECTION     *Section = &Context->Sections[Reloc->Section];

    if (Reloc->Type == MmixRelocPcRel32) {
      //
      // PC-relative branch
      //
      INT64   Offset = (Symbol->Value - Reloc->Offset - 4) / 4;
      UINT16  OffsetField = (UINT16)(Offset & 0xFFFF);

      Section->Data[Reloc->Offset + 2] = (OffsetField >> 8) & 0xFF;
      Section->Data[Reloc->Offset + 3] = OffsetField & 0xFF;
    }
  }

  return MMIX_SUCCESS;
}

/**
  Write object file in simple format.

  @param[in]      Context       Assembler context.
  @param[in]      FilePath      Output file path.

  @retval MMIX_SUCCESS          Object file written successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmWriteObject (
  IN  MMIX_ASM_CONTEXT  *Context,
  IN  CONST CHAR8       *FilePath
  )
{
  FILE  *File;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  File = fopen (FilePath, "wb");
  if (File == NULL) {
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Write .text section
  //
  if (Context->SectionCount > 0 && Context->Sections[0].Size > 0) {
    fwrite (Context->Sections[0].Data, 1, Context->Sections[0].Size, File);
  }

  fclose (File);
  return MMIX_SUCCESS;
}
