/** @file
  MMIX Librarian (Archive) tool implementation.

  This file implements an ar-like tool for creating and managing
  static library archives containing MMIX object files.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "../../include/MmixTypes.h"

//
// Archive file format (simplified ar format)
//
#define AR_MAGIC      "!<arch>\n"
#define AR_MAGIC_LEN  8

//
// Archive member header
//
typedef struct {
  CHAR8   Name[16];
  CHAR8   Date[12];
  CHAR8   Uid[6];
  CHAR8   Gid[6];
  CHAR8   Mode[8];
  CHAR8   Size[10];
  CHAR8   Magic[2];
} AR_HEADER;

#define AR_HEADER_MAGIC  "`\n"

/**
  Archive operation modes
**/
typedef enum {
  ArModeCreate,
  ArModeExtract,
  ArModeList,
  ArModeDelete,
  ArModeReplace
} AR_MODE;

/**
  Archive context
**/
typedef struct {
  AR_MODE  Mode;
  CHAR8    *ArchiveFile;
  CHAR8    **MemberFiles;
  UINT32   MemberCount;
  BOOLEAN  Verbose;
  BOOLEAN  CreateArchive;
} AR_CONTEXT;

/**
  Format archive header.

  @param[out]     Header        Pointer to header structure.
  @param[in]      Name          Member name.
  @param[in]      Size          Member size.

**/
STATIC
VOID
FormatArchiveHeader (
  OUT AR_HEADER    *Header,
  IN  CONST CHAR8  *Name,
  IN  UINT64       Size
  )
{
  time_t      Now;
  struct stat St;

  memset (Header, ' ', sizeof (AR_HEADER));

  //
  // Format name (truncate if necessary)
  //
  snprintf (Header->Name, sizeof (Header->Name), "%-16s", Name);

  //
  // Format timestamp
  //
  Now = time (NULL);
  snprintf (Header->Date, sizeof (Header->Date), "%-12lu", (unsigned long)Now);

  //
  // Format UIDs (use dummy values)
  //
  snprintf (Header->Uid, sizeof (Header->Uid), "%-6u", 0);
  snprintf (Header->Gid, sizeof (Header->Gid), "%-6u", 0);

  //
  // Format mode
  //
  snprintf (Header->Mode, sizeof (Header->Mode), "%-8o", 0644);

  //
  // Format size
  //
  snprintf (Header->Size, sizeof (Header->Size), "%-10llu", (unsigned long long)Size);

  //
  // Magic bytes
  //
  memcpy (Header->Magic, AR_HEADER_MAGIC, 2);
}

/**
  Parse size from archive header.

  @param[in]      Header        Pointer to header.

  @return  Parsed size value.

**/
STATIC
UINT64
ParseHeaderSize (
  IN  AR_HEADER  *Header
  )
{
  CHAR8  SizeStr[11];

  memcpy (SizeStr, Header->Size, 10);
  SizeStr[10] = '\0';

  return strtoull (SizeStr, NULL, 10);
}

/**
  Create a new archive or replace members.

  @param[in]      Context       Archive context.

  @retval MMIX_SUCCESS          Operation successful.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
CreateOrReplaceArchive (
  IN  AR_CONTEXT  *Context
  )
{
  FILE        *ArchiveFile;
  FILE        *MemberFile;
  AR_HEADER   Header;
  UINT8       *Buffer;
  size_t      FileSize;
  struct stat St;

  //
  // Create archive file
  //
  ArchiveFile = fopen (Context->ArchiveFile, "wb");
  if (ArchiveFile == NULL) {
    fprintf (stderr, "Error: Cannot create archive: %s\n", Context->ArchiveFile);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Write magic
  //
  fwrite (AR_MAGIC, 1, AR_MAGIC_LEN, ArchiveFile);

  //
  // Add members
  //
  for (UINT32 i = 0; i < Context->MemberCount; i++) {
    //
    // Get file size
    //
    if (stat (Context->MemberFiles[i], &St) != 0) {
      fprintf (stderr, "Error: Cannot stat file: %s\n", Context->MemberFiles[i]);
      fclose (ArchiveFile);
      return MMIX_ERROR_NOT_FOUND;
    }

    FileSize = St.st_size;

    //
    // Format header
    //
    FormatArchiveHeader (&Header, Context->MemberFiles[i], FileSize);

    //
    // Write header
    //
    fwrite (&Header, 1, sizeof (AR_HEADER), ArchiveFile);

    //
    // Read and write member data
    //
    MemberFile = fopen (Context->MemberFiles[i], "rb");
    if (MemberFile == NULL) {
      fprintf (stderr, "Error: Cannot open file: %s\n", Context->MemberFiles[i]);
      fclose (ArchiveFile);
      return MMIX_ERROR_NOT_FOUND;
    }

    Buffer = (UINT8 *)malloc (FileSize);
    if (Buffer == NULL) {
      fprintf (stderr, "Error: Out of memory\n");
      fclose (MemberFile);
      fclose (ArchiveFile);
      return MMIX_ERROR_OUT_OF_MEMORY;
    }

    if (fread (Buffer, 1, FileSize, MemberFile) != FileSize) {
      fprintf (stderr, "Error: Cannot read file: %s\n", Context->MemberFiles[i]);
      free (Buffer);
      fclose (MemberFile);
      fclose (ArchiveFile);
      return MMIX_ERROR_DEVICE_ERROR;
    }

    fwrite (Buffer, 1, FileSize, ArchiveFile);

    //
    // Pad to even boundary
    //
    if (FileSize & 1) {
      fputc ('\n', ArchiveFile);
    }

    free (Buffer);
    fclose (MemberFile);

    if (Context->Verbose) {
      printf ("a - %s\n", Context->MemberFiles[i]);
    }
  }

  fclose (ArchiveFile);
  return MMIX_SUCCESS;
}

/**
  List archive contents.

  @param[in]      Context       Archive context.

  @retval MMIX_SUCCESS          Operation successful.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
ListArchive (
  IN  AR_CONTEXT  *Context
  )
{
  FILE       *ArchiveFile;
  CHAR8      Magic[AR_MAGIC_LEN];
  AR_HEADER  Header;
  UINT64     MemberSize;

  //
  // Open archive
  //
  ArchiveFile = fopen (Context->ArchiveFile, "rb");
  if (ArchiveFile == NULL) {
    fprintf (stderr, "Error: Cannot open archive: %s\n", Context->ArchiveFile);
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Read and verify magic
  //
  if (fread (Magic, 1, AR_MAGIC_LEN, ArchiveFile) != AR_MAGIC_LEN ||
      memcmp (Magic, AR_MAGIC, AR_MAGIC_LEN) != 0) {
    fprintf (stderr, "Error: Not a valid archive file\n");
    fclose (ArchiveFile);
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Read members
  //
  while (fread (&Header, 1, sizeof (AR_HEADER), ArchiveFile) == sizeof (AR_HEADER)) {
    //
    // Verify header magic
    //
    if (memcmp (Header.Magic, AR_HEADER_MAGIC, 2) != 0) {
      fprintf (stderr, "Error: Invalid member header\n");
      break;
    }

    //
    // Parse size
    //
    MemberSize = ParseHeaderSize (&Header);

    //
    // Print member name
    //
    if (Context->Verbose) {
      CHAR8  Name[17];
      memcpy (Name, Header.Name, 16);
      Name[16] = '\0';
      //
      // Trim trailing spaces
      //
      for (INT32 i = 15; i >= 0 && Name[i] == ' '; i--) {
        Name[i] = '\0';
      }
      printf ("%-16s  %10llu bytes\n", Name, (unsigned long long)MemberSize);
    } else {
      printf ("%.16s\n", Header.Name);
    }

    //
    // Skip member data
    //
    if (fseek (ArchiveFile, MemberSize + (MemberSize & 1), SEEK_CUR) != 0) {
      break;
    }
  }

  fclose (ArchiveFile);
  return MMIX_SUCCESS;
}

/**
  Extract archive members.

  @param[in]      Context       Archive context.

  @retval MMIX_SUCCESS          Operation successful.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
ExtractArchive (
  IN  AR_CONTEXT  *Context
  )
{
  FILE       *ArchiveFile;
  FILE       *OutputFile;
  CHAR8      Magic[AR_MAGIC_LEN];
  AR_HEADER  Header;
  UINT64     MemberSize;
  UINT8      *Buffer;
  CHAR8      Name[17];

  //
  // Open archive
  //
  ArchiveFile = fopen (Context->ArchiveFile, "rb");
  if (ArchiveFile == NULL) {
    fprintf (stderr, "Error: Cannot open archive: %s\n", Context->ArchiveFile);
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Read and verify magic
  //
  if (fread (Magic, 1, AR_MAGIC_LEN, ArchiveFile) != AR_MAGIC_LEN ||
      memcmp (Magic, AR_MAGIC, AR_MAGIC_LEN) != 0) {
    fprintf (stderr, "Error: Not a valid archive file\n");
    fclose (ArchiveFile);
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Extract members
  //
  while (fread (&Header, 1, sizeof (AR_HEADER), ArchiveFile) == sizeof (AR_HEADER)) {
    //
    // Verify header magic
    //
    if (memcmp (Header.Magic, AR_HEADER_MAGIC, 2) != 0) {
      fprintf (stderr, "Error: Invalid member header\n");
      break;
    }

    //
    // Parse name and size
    //
    memcpy (Name, Header.Name, 16);
    Name[16] = '\0';
    for (INT32 i = 15; i >= 0 && Name[i] == ' '; i--) {
      Name[i] = '\0';
    }

    MemberSize = ParseHeaderSize (&Header);

    //
    // Check if we should extract this member
    //
    BOOLEAN  Extract = TRUE;
    if (Context->MemberCount > 0) {
      Extract = FALSE;
      for (UINT32 i = 0; i < Context->MemberCount; i++) {
        if (strcmp (Name, Context->MemberFiles[i]) == 0) {
          Extract = TRUE;
          break;
        }
      }
    }

    if (Extract) {
      //
      // Read member data
      //
      Buffer = (UINT8 *)malloc (MemberSize);
      if (Buffer == NULL) {
        fprintf (stderr, "Error: Out of memory\n");
        fclose (ArchiveFile);
        return MMIX_ERROR_OUT_OF_MEMORY;
      }

      if (fread (Buffer, 1, MemberSize, ArchiveFile) != MemberSize) {
        fprintf (stderr, "Error: Cannot read member data\n");
        free (Buffer);
        fclose (ArchiveFile);
        return MMIX_ERROR_DEVICE_ERROR;
      }

      //
      // Write to output file
      //
      OutputFile = fopen (Name, "wb");
      if (OutputFile == NULL) {
        fprintf (stderr, "Error: Cannot create file: %s\n", Name);
        free (Buffer);
        fclose (ArchiveFile);
        return MMIX_ERROR_DEVICE_ERROR;
      }

      fwrite (Buffer, 1, MemberSize, OutputFile);
      fclose (OutputFile);
      free (Buffer);

      if (Context->Verbose) {
        printf ("x - %s\n", Name);
      }

      //
      // Skip padding
      //
      if (MemberSize & 1) {
        fseek (ArchiveFile, 1, SEEK_CUR);
      }
    } else {
      //
      // Skip member
      //
      fseek (ArchiveFile, MemberSize + (MemberSize & 1), SEEK_CUR);
    }
  }

  fclose (ArchiveFile);
  return MMIX_SUCCESS;
}

/**
  Main librarian entry point.

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
  AR_CONTEXT   Context;
  MMIX_STATUS  Status;

  if (argc < 3) {
    printf ("Usage: mmix-ar [options] archive [members...]\n");
    printf ("Options:\n");
    printf ("  r  - Replace or add members (create archive)\n");
    printf ("  t  - List archive contents\n");
    printf ("  x  - Extract members\n");
    printf ("  d  - Delete members\n");
    printf ("  v  - Verbose mode\n");
    printf ("  c  - Create archive (suppress warning)\n");
    return 1;
  }

  //
  // Initialize context
  //
  memset (&Context, 0, sizeof (Context));
  Context.Mode = ArModeCreate;
  Context.Verbose = FALSE;
  Context.CreateArchive = FALSE;

  //
  // Parse options
  //
  CONST CHAR8  *Options = argv[1];
  for (CONST CHAR8 *p = Options; *p != '\0'; p++) {
    switch (*p) {
      case 'r':
        Context.Mode = ArModeReplace;
        break;
      case 't':
        Context.Mode = ArModeList;
        break;
      case 'x':
        Context.Mode = ArModeExtract;
        break;
      case 'd':
        Context.Mode = ArModeDelete;
        break;
      case 'v':
        Context.Verbose = TRUE;
        break;
      case 'c':
        Context.CreateArchive = TRUE;
        break;
    }
  }

  //
  // Get archive file name
  //
  Context.ArchiveFile = argv[2];

  //
  // Get member files
  //
  if (argc > 3) {
    Context.MemberFiles = &argv[3];
    Context.MemberCount = argc - 3;
  }

  //
  // Perform operation
  //
  switch (Context.Mode) {
    case ArModeReplace:
    case ArModeCreate:
      Status = CreateOrReplaceArchive (&Context);
      break;

    case ArModeList:
      Status = ListArchive (&Context);
      break;

    case ArModeExtract:
      Status = ExtractArchive (&Context);
      break;

    case ArModeDelete:
      fprintf (stderr, "Error: Delete operation not yet implemented\n");
      Status = MMIX_ERROR_UNSUPPORTED;
      break;

    default:
      fprintf (stderr, "Error: Invalid operation mode\n");
      Status = MMIX_ERROR_INVALID_PARAMETER;
      break;
  }

  if (Status != MMIX_SUCCESS) {
    return 1;
  }

  return 0;
}
