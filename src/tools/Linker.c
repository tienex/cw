/** @file
  MMIX Linker implementation.

  This file implements the linker for combining MMIX object files
  into executables.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../../include/MmixLinker.h"
#include "../../include/MmixAsm.h"

/**
  Create a new linker context.

  @param[out]     Context       Pointer to receive context.

  @retval MMIX_SUCCESS          Context created successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerCreate (
  OUT MMIX_LINKER_CONTEXT  **Context
  )
{
  MMIX_LINKER_CONTEXT  *Ctx;

  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Ctx = (MMIX_LINKER_CONTEXT *)calloc (1, sizeof (MMIX_LINKER_CONTEXT));
  if (Ctx == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Ctx->OutputFormat = MmixOutputRaw;
  Ctx->BaseAddress = 0;
  Ctx->EntryPoint = 0;
  Ctx->Relocatable = FALSE;

  *Context = Ctx;
  return MMIX_SUCCESS;
}

/**
  Destroy a linker context.

  @param[in]      Context       Linker context.

**/
VOID
MmixLinkerDestroy (
  IN  MMIX_LINKER_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return;
  }

  //
  // Free input file names
  //
  for (UINT32 i = 0; i < Context->InputFileCount; i++) {
    if (Context->InputFiles[i] != NULL) {
      free (Context->InputFiles[i]);
    }
  }

  //
  // Free assembler contexts
  //
  for (UINT32 i = 0; i < Context->ObjectCount; i++) {
    if (Context->Objects[i] != NULL) {
      MmixAsmDestroy (Context->Objects[i]);
    }
  }

  if (Context->OutputFile != NULL) {
    free (Context->OutputFile);
  }

  free (Context);
}

/**
  Add an input file to the linker.

  @param[in,out]  Context       Linker context.
  @param[in]      FilePath      Path to input file.

  @retval MMIX_SUCCESS          File added successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerAddInput (
  IN OUT MMIX_LINKER_CONTEXT  *Context,
  IN     CONST CHAR8          *FilePath
  )
{
  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->InputFileCount >= MMIX_MAX_INPUT_FILES) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Context->InputFiles[Context->InputFileCount] = strdup (FilePath);
  if (Context->InputFiles[Context->InputFileCount] == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Context->InputFileCount++;
  return MMIX_SUCCESS;
}

/**
  Set linker output options.

  @param[in,out]  Context       Linker context.
  @param[in]      OutputFile    Output file path.
  @param[in]      Format        Output format.
  @param[in]      BaseAddress   Base address for output.

  @retval MMIX_SUCCESS          Options set successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerSetOutput (
  IN OUT MMIX_LINKER_CONTEXT  *Context,
  IN     CONST CHAR8          *OutputFile,
  IN     MMIX_OUTPUT_FORMAT   Format,
  IN     UINT64               BaseAddress
  )
{
  if (Context == NULL || OutputFile == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->OutputFile != NULL) {
    free (Context->OutputFile);
  }

  Context->OutputFile = strdup (OutputFile);
  if (Context->OutputFile == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Context->OutputFormat = Format;
  Context->BaseAddress = BaseAddress;

  return MMIX_SUCCESS;
}

/**
  Resolve symbols across all input objects.

  @param[in,out]  Context       Linker context.

  @retval MMIX_SUCCESS          Resolution successful.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
ResolveSymbols (
  IN OUT MMIX_LINKER_CONTEXT  *Context
  )
{
  //
  // Build global symbol table
  //
  for (UINT32 i = 0; i < Context->ObjectCount; i++) {
    MMIX_ASM_CONTEXT  *Obj = Context->Objects[i];

    for (UINT32 j = 0; j < Obj->SymbolCount; j++) {
      MMIX_SYMBOL  *Sym = &Obj->Symbols[j];

      if (Sym->Bind == MmixBindGlobal || Sym->Bind == MmixBindWeak) {
        //
        // Check for duplicate global symbols
        //
        BOOLEAN  Found = FALSE;
        for (UINT32 k = 0; k < Context->GlobalSymbolCount; k++) {
          if (strcmp (Context->GlobalSymbols[k].Name, Sym->Name) == 0) {
            if (Context->GlobalSymbols[k].Bind == MmixBindWeak) {
              //
              // Replace weak symbol with strong one
              //
              Context->GlobalSymbols[k] = *Sym;
            } else if (Sym->Bind != MmixBindWeak) {
              fprintf (stderr, "Error: Multiple definitions of symbol '%s'\n", Sym->Name);
              return MMIX_ERROR_INVALID_PARAMETER;
            }
            Found = TRUE;
            break;
          }
        }

        if (!Found) {
          if (Context->GlobalSymbolCount >= MMIX_MAX_SYMBOLS) {
            return MMIX_ERROR_OUT_OF_MEMORY;
          }
          Context->GlobalSymbols[Context->GlobalSymbolCount++] = *Sym;
        }
      }
    }
  }

  //
  // Check for undefined symbols
  //
  for (UINT32 i = 0; i < Context->ObjectCount; i++) {
    MMIX_ASM_CONTEXT  *Obj = Context->Objects[i];

    for (UINT32 j = 0; j < Obj->RelocationCount; j++) {
      MMIX_RELOCATION  *Reloc = &Obj->Relocations[j];
      MMIX_SYMBOL      *Sym = &Obj->Symbols[Reloc->Symbol];

      if (!Sym->Defined && Sym->Type != MmixSymbolExternal) {
        //
        // Check if defined in another object
        //
        BOOLEAN  Found = FALSE;
        for (UINT32 k = 0; k < Context->GlobalSymbolCount; k++) {
          if (strcmp (Context->GlobalSymbols[k].Name, Sym->Name) == 0 &&
              Context->GlobalSymbols[k].Defined) {
            Found = TRUE;
            break;
          }
        }

        if (!Found) {
          fprintf (stderr, "Error: Undefined symbol '%s'\n", Sym->Name);
          return MMIX_ERROR_NOT_FOUND;
        }
      }
    }
  }

  return MMIX_SUCCESS;
}

/**
  Merge sections from all input objects.

  @param[in,out]  Context       Linker context.
  @param[out]     OutputData    Pointer to receive merged data.
  @param[out]     OutputSize    Pointer to receive output size.

  @retval MMIX_SUCCESS          Merge successful.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
MergeSections (
  IN OUT MMIX_LINKER_CONTEXT  *Context,
  OUT    UINT8                **OutputData,
  OUT    UINT64               *OutputSize
  )
{
  UINT64  TotalSize;
  UINT8   *Data;
  UINT64  Offset;

  //
  // Calculate total size
  //
  TotalSize = 0;
  for (UINT32 i = 0; i < Context->ObjectCount; i++) {
    MMIX_ASM_CONTEXT  *Obj = Context->Objects[i];
    for (UINT32 j = 0; j < Obj->SectionCount; j++) {
      TotalSize += Obj->Sections[j].Size;
      //
      // Align to 8 bytes
      //
      if ((TotalSize & 7) != 0) {
        TotalSize = (TotalSize + 7) & ~7ULL;
      }
    }
  }

  //
  // Allocate output buffer
  //
  Data = (UINT8 *)calloc (1, TotalSize);
  if (Data == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  //
  // Merge sections
  //
  Offset = 0;
  for (UINT32 i = 0; i < Context->ObjectCount; i++) {
    MMIX_ASM_CONTEXT  *Obj = Context->Objects[i];
    for (UINT32 j = 0; j < Obj->SectionCount; j++) {
      MMIX_SECTION  *Sec = &Obj->Sections[j];

      if (Sec->Size > 0 && Sec->Data != NULL) {
        memcpy (Data + Offset, Sec->Data, Sec->Size);
        Sec->Address = Context->BaseAddress + Offset;
        Offset += Sec->Size;

        //
        // Align to 8 bytes
        //
        if ((Offset & 7) != 0) {
          Offset = (Offset + 7) & ~7ULL;
        }
      }
    }
  }

  *OutputData = Data;
  *OutputSize = Offset;
  return MMIX_SUCCESS;
}

/**
  Apply relocations to the merged output.

  @param[in,out]  Context       Linker context.
  @param[in,out]  Data          Output data buffer.
  @param[in]      Size          Size of output data.

  @retval MMIX_SUCCESS          Relocations applied successfully.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
ApplyRelocations (
  IN OUT MMIX_LINKER_CONTEXT  *Context,
  IN OUT UINT8                *Data,
  IN     UINT64               Size
  )
{
  for (UINT32 i = 0; i < Context->ObjectCount; i++) {
    MMIX_ASM_CONTEXT  *Obj = Context->Objects[i];

    for (UINT32 j = 0; j < Obj->RelocationCount; j++) {
      MMIX_RELOCATION  *Reloc = &Obj->Relocations[j];
      MMIX_SYMBOL      *LocalSym = &Obj->Symbols[Reloc->Symbol];
      UINT64           TargetValue = 0;

      //
      // Find symbol value
      //
      if (LocalSym->Defined) {
        TargetValue = LocalSym->Value;
      } else {
        //
        // Look in global symbols
        //
        for (UINT32 k = 0; k < Context->GlobalSymbolCount; k++) {
          if (strcmp (Context->GlobalSymbols[k].Name, LocalSym->Name) == 0) {
            TargetValue = Context->GlobalSymbols[k].Value;
            break;
          }
        }
      }

      //
      // Apply relocation
      //
      MMIX_SECTION  *Sec = &Obj->Sections[Reloc->Section];
      UINT64        PatchOffset = Sec->Address - Context->BaseAddress + Reloc->Offset;

      if (PatchOffset + 4 > Size) {
        fprintf (stderr, "Error: Relocation offset out of bounds\n");
        return MMIX_ERROR_INVALID_PARAMETER;
      }

      if (Reloc->Type == MmixRelocPcRel32) {
        //
        // PC-relative relocation
        //
        INT64   Offset = (TargetValue - (Sec->Address + Reloc->Offset)) / 4;
        UINT16  OffsetField = (UINT16)(Offset & 0xFFFF);

        Data[PatchOffset + 2] = (OffsetField >> 8) & 0xFF;
        Data[PatchOffset + 3] = OffsetField & 0xFF;
      } else if (Reloc->Type == MmixRelocAbsolute64) {
        //
        // Absolute 64-bit relocation
        //
        for (INT32 k = 0; k < 8; k++) {
          Data[PatchOffset + k] = (TargetValue >> (56 - k * 8)) & 0xFF;
        }
      }
    }
  }

  return MMIX_SUCCESS;
}

/**
  Perform the link operation.

  @param[in,out]  Context       Linker context.

  @retval MMIX_SUCCESS          Link successful.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerLink (
  IN OUT MMIX_LINKER_CONTEXT  *Context
  )
{
  MMIX_STATUS  Status;
  UINT8        *OutputData;
  UINT64       OutputSize;
  FILE         *OutFile;

  if (Context == NULL || Context->OutputFile == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Load all input files
  //
  for (UINT32 i = 0; i < Context->InputFileCount; i++) {
    MMIX_ASM_CONTEXT  *Obj;

    Status = MmixAsmCreate (&Obj);
    if (Status != MMIX_SUCCESS) {
      return Status;
    }

    Status = MmixAsmAssembleFile (Obj, Context->InputFiles[i]);
    if (Status != MMIX_SUCCESS) {
      fprintf (stderr, "Error assembling %s\n", Context->InputFiles[i]);
      MmixAsmDestroy (Obj);
      return Status;
    }

    Context->Objects[Context->ObjectCount++] = Obj;
  }

  //
  // Resolve symbols
  //
  Status = ResolveSymbols (Context);
  if (Status != MMIX_SUCCESS) {
    return Status;
  }

  //
  // Merge sections
  //
  Status = MergeSections (Context, &OutputData, &OutputSize);
  if (Status != MMIX_SUCCESS) {
    return Status;
  }

  //
  // Apply relocations
  //
  Status = ApplyRelocations (Context, OutputData, OutputSize);
  if (Status != MMIX_SUCCESS) {
    free (OutputData);
    return Status;
  }

  //
  // Write output file
  //
  OutFile = fopen (Context->OutputFile, "wb");
  if (OutFile == NULL) {
    fprintf (stderr, "Error: Cannot create output file: %s\n", Context->OutputFile);
    free (OutputData);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  if (fwrite (OutputData, 1, OutputSize, OutFile) != OutputSize) {
    fprintf (stderr, "Error: Cannot write output file\n");
    fclose (OutFile);
    free (OutputData);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  fclose (OutFile);
  free (OutputData);

  printf ("Linked %u objects into %s (%llu bytes)\n",
         Context->ObjectCount, Context->OutputFile, (unsigned long long)OutputSize);

  return MMIX_SUCCESS;
}

/**
  Main linker entry point.

  @param[in]      argc          Argument count.
  @param[in]      argv          Argument vector.

  @return  Exit status.

**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  MMIX_LINKER_CONTEXT  *Context;
  MMIX_STATUS          Status;
  CHAR8                *OutputFile;
  UINT64               BaseAddress;
  INT32                Opt;

  OutputFile = (CHAR8 *)"a.out";
  BaseAddress = 0;

  //
  // Parse options
  //
  while ((Opt = getopt (argc, argv, "o:b:")) != -1) {
    switch (Opt) {
      case 'o':
        OutputFile = optarg;
        break;
      case 'b':
        BaseAddress = strtoull (optarg, NULL, 0);
        break;
      default:
        fprintf (stderr, "Usage: mmix-ld [-o output] [-b base_address] file...\n");
        return 1;
    }
  }

  if (optind >= argc) {
    fprintf (stderr, "Error: No input files\n");
    return 1;
  }

  //
  // Create linker context
  //
  Status = MmixLinkerCreate (&Context);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Cannot create linker context\n");
    return 1;
  }

  //
  // Add input files
  //
  for (INT32 i = optind; i < argc; i++) {
    Status = MmixLinkerAddInput (Context, argv[i]);
    if (Status != MMIX_SUCCESS) {
      fprintf (stderr, "Error: Cannot add input file: %s\n", argv[i]);
      MmixLinkerDestroy (Context);
      return 1;
    }
  }

  //
  // Set output options
  //
  Status = MmixLinkerSetOutput (Context, OutputFile, MmixOutputRaw, BaseAddress);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Cannot set output options\n");
    MmixLinkerDestroy (Context);
    return 1;
  }

  //
  // Perform link
  //
  Status = MmixLinkerLink (Context);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Link failed\n");
    MmixLinkerDestroy (Context);
    return 1;
  }

  MmixLinkerDestroy (Context);
  return 0;
}
