/** @file
  Binary Format Input/Output Buffer Management Implementation.

  This file implements the unified buffer/file abstraction layer that allows
  binary format backends to work with files, memory-mapped files, buffers,
  and streaming I/O without duplicating code.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include "../../include/binformat/BinFormat.h"

//
// Default initial size for allocated buffers
//
#define DEFAULT_ALLOCATED_SIZE  (4096)

//
// Growth factor for buffer reallocation (1.5x)
//
#define GROWTH_FACTOR_NUM  3
#define GROWTH_FACTOR_DEN  2

/**
  Initialize input from file path (loads entire file into memory).

  This reads the entire file into memory. For large files or memory-constrained
  systems, consider using BinFormatInputInitFileMmap or BinFormatInputInitFileStream.

  @param[out]  Input             Pointer to input structure.
  @param[in]   FilePath          Path to file.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval BINFORMAT_SUCCESS      Input initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputInitFile(
  OUT BINFORMAT_INPUT  *Input,
  IN  CONST CHAR8      *FilePath,
  IN  BOOLEAN          ReadOnly
  )
{
  FILE    *File;
  UINT64  FileSize;
  UINT8   *Buffer;
  size_t  BytesRead;

  if (Input == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  File = fopen (FilePath, ReadOnly ? "rb" : "r+b");
  if (File == NULL) {
    return BINFORMAT_ERROR_IO;
  }

  //
  // Get file size
  //
  fseek (File, 0, SEEK_END);
  FileSize = (UINT64)ftell (File);
  fseek (File, 0, SEEK_SET);

  //
  // Allocate buffer
  //
  Buffer = (UINT8 *)malloc (FileSize);
  if (Buffer == NULL) {
    fclose (File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Read entire file
  //
  BytesRead = fread (Buffer, 1, (size_t)FileSize, File);
  fclose (File);

  if (BytesRead != (size_t)FileSize) {
    free (Buffer);
    return BINFORMAT_ERROR_IO;
  }

  //
  // Initialize structure
  //
  memset (Input, 0, sizeof (BINFORMAT_INPUT));
  Input->Type           = BinInputTypeFile;
  Input->FilePath       = strdup (FilePath);
  Input->Data           = Buffer;
  Input->Size           = FileSize;
  Input->Capacity       = FileSize;
  Input->ReadOnly       = ReadOnly;
  Input->OwnBuffer      = TRUE;
  Input->FileDescriptor = -1;

  return BINFORMAT_SUCCESS;
}

/**
  Initialize input from file using memory mapping.

  This uses mmap() to map the file into memory without loading it entirely.
  Good for large files on systems with virtual memory.

  @param[out]  Input             Pointer to input structure.
  @param[in]   FilePath          Path to file.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval BINFORMAT_SUCCESS      Input initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputInitFileMmap(
  OUT BINFORMAT_INPUT  *Input,
  IN  CONST CHAR8      *FilePath,
  IN  BOOLEAN          ReadOnly
  )
{
  INT32        Fd;
  struct stat  St;
  VOID         *MappedData;
  INT32        Prot;
  INT32        Flags;

  if (Input == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  Fd = open (FilePath, ReadOnly ? O_RDONLY : O_RDWR);
  if (Fd < 0) {
    return BINFORMAT_ERROR_IO;
  }

  //
  // Get file size
  //
  if (fstat (Fd, &St) < 0) {
    close (Fd);
    return BINFORMAT_ERROR_IO;
  }

  //
  // Map file into memory
  //
  Prot  = ReadOnly ? PROT_READ : (PROT_READ | PROT_WRITE);
  Flags = ReadOnly ? MAP_PRIVATE : MAP_SHARED;

  MappedData = mmap (NULL, (size_t)St.st_size, Prot, Flags, Fd, 0);
  if (MappedData == MAP_FAILED) {
    close (Fd);
    return BINFORMAT_ERROR_IO;
  }

  //
  // Initialize structure
  //
  memset (Input, 0, sizeof (BINFORMAT_INPUT));
  Input->Type           = BinInputTypeMmap;
  Input->FilePath       = strdup (FilePath);
  Input->Data           = (UINT8 *)MappedData;
  Input->Size           = (UINT64)St.st_size;
  Input->Capacity       = (UINT64)St.st_size;
  Input->ReadOnly       = ReadOnly;
  Input->OwnBuffer      = TRUE;
  Input->FileDescriptor = Fd;
  Input->MmapBase       = MappedData;

  return BINFORMAT_SUCCESS;
}

/**
  Initialize input from file using streaming I/O.

  This uses streaming file I/O for memory-constrained systems. Data is read
  on-demand using BinFormatInputRead() and written using BinFormatInputWrite().

  @param[out]  Input             Pointer to input structure.
  @param[in]   FilePath          Path to file.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval BINFORMAT_SUCCESS      Input initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputInitFileStream(
  OUT BINFORMAT_INPUT  *Input,
  IN  CONST CHAR8      *FilePath,
  IN  BOOLEAN          ReadOnly
  )
{
  INT32        Fd;
  struct stat  St;

  if (Input == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  Fd = open (FilePath, ReadOnly ? O_RDONLY : O_RDWR);
  if (Fd < 0) {
    return BINFORMAT_ERROR_IO;
  }

  //
  // Get file size
  //
  if (fstat (Fd, &St) < 0) {
    close (Fd);
    return BINFORMAT_ERROR_IO;
  }

  //
  // Initialize structure (no buffer for streaming mode)
  //
  memset (Input, 0, sizeof (BINFORMAT_INPUT));
  Input->Type           = BinInputTypeStream;
  Input->FilePath       = strdup (FilePath);
  Input->Data           = NULL;
  Input->Size           = (UINT64)St.st_size;
  Input->Capacity       = 0;
  Input->ReadOnly       = ReadOnly;
  Input->OwnBuffer      = FALSE;
  Input->FileDescriptor = Fd;
  Input->FilePosition   = 0;

  return BINFORMAT_SUCCESS;
}

/**
  Initialize input from existing buffer.

  @param[out]  Input             Pointer to input structure.
  @param[in]   Buffer            Pointer to buffer data.
  @param[in]   Size              Size of buffer.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval BINFORMAT_SUCCESS      Input initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputInitBuffer(
  OUT BINFORMAT_INPUT  *Input,
  IN  CONST VOID       *Buffer,
  IN  UINT64           Size,
  IN  BOOLEAN          ReadOnly
  )
{
  if (Input == NULL || Buffer == NULL || Size == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Initialize structure
  //
  memset (Input, 0, sizeof (BINFORMAT_INPUT));
  Input->Type           = BinInputTypeBuffer;
  Input->FilePath       = NULL;
  Input->Data           = (UINT8 *)Buffer;
  Input->Size           = Size;
  Input->Capacity       = Size;
  Input->ReadOnly       = ReadOnly;
  Input->OwnBuffer      = FALSE;
  Input->FileDescriptor = -1;

  return BINFORMAT_SUCCESS;
}

/**
  Initialize input with allocated buffer (for creating new binary images).

  @param[out]  Input             Pointer to input structure.
  @param[in]   InitialSize       Initial buffer size (0 for default).

  @retval BINFORMAT_SUCCESS      Input initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputInitAllocated(
  OUT BINFORMAT_INPUT  *Input,
  IN  UINT64           InitialSize
  )
{
  UINT8  *Buffer;
  UINT64 AllocSize;

  if (Input == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Use default size if not specified
  //
  AllocSize = (InitialSize == 0) ? DEFAULT_ALLOCATED_SIZE : InitialSize;

  //
  // Allocate buffer
  //
  Buffer = (UINT8 *)malloc ((size_t)AllocSize);
  if (Buffer == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  memset (Buffer, 0, (size_t)AllocSize);

  //
  // Initialize structure
  //
  memset (Input, 0, sizeof (BINFORMAT_INPUT));
  Input->Type           = BinInputTypeAllocated;
  Input->FilePath       = NULL;
  Input->Data           = Buffer;
  Input->Size           = 0;  // No data yet, just allocated space
  Input->Capacity       = AllocSize;
  Input->ReadOnly       = FALSE;
  Input->OwnBuffer      = TRUE;
  Input->FileDescriptor = -1;

  return BINFORMAT_SUCCESS;
}

/**
  Read data from input at specified offset.

  Works with all input types. For buffered types (File/Buffer/Mmap/Allocated),
  this is a simple memory copy. For streaming types, this performs a file seek
  and read operation.

  @param[in]  Input              Input structure.
  @param[in]  Offset             Offset to read from.
  @param[out] Buffer             Buffer to read into.
  @param[in]  Size               Number of bytes to read.

  @retval BINFORMAT_SUCCESS      Data read successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputRead(
  IN  BINFORMAT_INPUT  *Input,
  IN  UINT64           Offset,
  OUT VOID             *Buffer,
  IN  UINT64           Size
  )
{
  ssize_t  BytesRead;

  if (Input == NULL || Buffer == NULL || Size == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Check bounds
  //
  if (Offset + Size > Input->Size) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Handle based on input type
  //
  switch (Input->Type) {
    case BinInputTypeFile:
    case BinInputTypeBuffer:
    case BinInputTypeMmap:
    case BinInputTypeAllocated:
      //
      // Simple memory copy for buffered types
      //
      memcpy (Buffer, Input->Data + Offset, (size_t)Size);
      return BINFORMAT_SUCCESS;

    case BinInputTypeStream:
      //
      // File seek and read for streaming
      //
      if (lseek (Input->FileDescriptor, (off_t)Offset, SEEK_SET) < 0) {
        return BINFORMAT_ERROR_IO;
      }

      BytesRead = read (Input->FileDescriptor, Buffer, (size_t)Size);
      if (BytesRead != (ssize_t)Size) {
        return BINFORMAT_ERROR_IO;
      }

      Input->FilePosition = Offset + Size;
      return BINFORMAT_SUCCESS;

    default:
      return BINFORMAT_ERROR_INVALID_PARAMETER;
  }
}

/**
  Write data to input at specified offset.

  Works with writable input types. For buffered types, this is a memory copy.
  For streaming types, this performs a file seek and write operation.
  For allocated types, automatically resizes the buffer if needed.

  @param[in]  Input              Input structure.
  @param[in]  Offset             Offset to write to.
  @param[in]  Buffer             Buffer to write from.
  @param[in]  Size               Number of bytes to write.

  @retval BINFORMAT_SUCCESS      Data written successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputWrite(
  IN  BINFORMAT_INPUT  *Input,
  IN  UINT64           Offset,
  IN  CONST VOID       *Buffer,
  IN  UINT64           Size
  )
{
  BINFORMAT_STATUS  Status;
  ssize_t           BytesWritten;

  if (Input == NULL || Buffer == NULL || Size == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Check if writable
  //
  if (Input->ReadOnly) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Handle based on input type
  //
  switch (Input->Type) {
    case BinInputTypeFile:
    case BinInputTypeBuffer:
    case BinInputTypeMmap:
      //
      // Check bounds for non-resizable types
      //
      if (Offset + Size > Input->Size) {
        return BINFORMAT_ERROR_BUFFER_TOO_SMALL;
      }

      memcpy (Input->Data + Offset, Buffer, (size_t)Size);
      return BINFORMAT_SUCCESS;

    case BinInputTypeAllocated:
      //
      // Auto-resize if needed
      //
      if (Offset + Size > Input->Capacity) {
        UINT64  NewCapacity;

        NewCapacity = Offset + Size;
        //
        // Grow by 1.5x to avoid frequent reallocations
        //
        NewCapacity = (NewCapacity * GROWTH_FACTOR_NUM) / GROWTH_FACTOR_DEN;

        Status = BinFormatInputResize (Input, NewCapacity);
        if (BINFORMAT_IS_ERROR (Status)) {
          return Status;
        }
      }

      memcpy (Input->Data + Offset, Buffer, (size_t)Size);

      //
      // Update size if we wrote beyond current end
      //
      if (Offset + Size > Input->Size) {
        Input->Size = Offset + Size;
      }

      return BINFORMAT_SUCCESS;

    case BinInputTypeStream:
      //
      // Check bounds
      //
      if (Offset + Size > Input->Size) {
        //
        // For streaming, we can extend the file
        //
        Input->Size = Offset + Size;
      }

      //
      // File seek and write
      //
      if (lseek (Input->FileDescriptor, (off_t)Offset, SEEK_SET) < 0) {
        return BINFORMAT_ERROR_IO;
      }

      BytesWritten = write (Input->FileDescriptor, Buffer, (size_t)Size);
      if (BytesWritten != (ssize_t)Size) {
        return BINFORMAT_ERROR_IO;
      }

      Input->FilePosition = Offset + Size;
      return BINFORMAT_SUCCESS;

    default:
      return BINFORMAT_ERROR_INVALID_PARAMETER;
  }
}

/**
  Resize an allocated buffer.

  Only works with BinInputTypeAllocated. For other types, returns error.

  @param[in]  Input              Input structure.
  @param[in]  NewSize            New buffer size.

  @retval BINFORMAT_SUCCESS      Buffer resized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputResize(
  IN  BINFORMAT_INPUT  *Input,
  IN  UINT64           NewSize
  )
{
  UINT8  *NewBuffer;

  if (Input == NULL || NewSize == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Only works with allocated buffers
  //
  if (Input->Type != BinInputTypeAllocated) {
    return BINFORMAT_ERROR_UNSUPPORTED;
  }

  //
  // Check if resize is needed
  //
  if (NewSize <= Input->Capacity) {
    //
    // No reallocation needed, just update size
    //
    if (NewSize > Input->Size) {
      //
      // Growing within capacity, zero new space
      //
      memset (Input->Data + Input->Size, 0, (size_t)(NewSize - Input->Size));
    }
    Input->Size = NewSize;
    return BINFORMAT_SUCCESS;
  }

  //
  // Reallocate buffer
  //
  NewBuffer = (UINT8 *)realloc (Input->Data, (size_t)NewSize);
  if (NewBuffer == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Zero new space
  //
  if (NewSize > Input->Size) {
    memset (NewBuffer + Input->Size, 0, (size_t)(NewSize - Input->Size));
  }

  Input->Data     = NewBuffer;
  Input->Size     = NewSize;
  Input->Capacity = NewSize;

  return BINFORMAT_SUCCESS;
}

/**
  Flush any pending writes to disk.

  For streaming mode, ensures all buffered writes are written to disk.
  For memory-mapped files, performs msync().
  For other modes, this is a no-op.

  @param[in]  Input              Input structure.

  @retval BINFORMAT_SUCCESS      Data flushed successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatInputFlush(
  IN  BINFORMAT_INPUT  *Input
  )
{
  if (Input == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  switch (Input->Type) {
    case BinInputTypeStream:
      //
      // Flush file descriptor
      //
      if (fsync (Input->FileDescriptor) < 0) {
        return BINFORMAT_ERROR_IO;
      }
      return BINFORMAT_SUCCESS;

    case BinInputTypeMmap:
      //
      // Sync memory-mapped region
      //
      if (!Input->ReadOnly) {
        if (msync (Input->MmapBase, (size_t)Input->Size, MS_SYNC) < 0) {
          return BINFORMAT_ERROR_IO;
        }
      }
      return BINFORMAT_SUCCESS;

    default:
      //
      // No-op for other types
      //
      return BINFORMAT_SUCCESS;
  }
}

/**
  Get direct pointer to data (if available).

  For buffered types (File/Buffer/Mmap/Allocated), returns pointer to data.
  For streaming types, returns NULL (must use BinFormatInputRead/Write).

  @param[in]  Input              Input structure.

  @return Pointer to data, or NULL if not available.
**/
VOID *
BinFormatInputGetDataPointer(
  IN  BINFORMAT_INPUT  *Input
  )
{
  if (Input == NULL) {
    return NULL;
  }

  //
  // Return NULL for streaming mode (no direct access)
  //
  if (Input->Type == BinInputTypeStream) {
    return NULL;
  }

  return Input->Data;
}

/**
  Close and free input resources.

  Closes files, unmaps memory, frees buffers as appropriate for the input type.

  @param[in]  Input              Input structure.
**/
VOID
BinFormatInputClose(
  IN  BINFORMAT_INPUT  *Input
  )
{
  if (Input == NULL) {
    return;
  }

  //
  // Free file path if allocated
  //
  if (Input->FilePath != NULL) {
    free (Input->FilePath);
    Input->FilePath = NULL;
  }

  //
  // Handle type-specific cleanup
  //
  switch (Input->Type) {
    case BinInputTypeFile:
    case BinInputTypeAllocated:
      //
      // Free buffer if we own it
      //
      if (Input->OwnBuffer && Input->Data != NULL) {
        free (Input->Data);
      }
      break;

    case BinInputTypeMmap:
      //
      // Unmap and close file
      //
      if (Input->MmapBase != NULL) {
        munmap (Input->MmapBase, (size_t)Input->Size);
      }
      if (Input->FileDescriptor >= 0) {
        close (Input->FileDescriptor);
      }
      break;

    case BinInputTypeStream:
      //
      // Close file descriptor
      //
      if (Input->FileDescriptor >= 0) {
        close (Input->FileDescriptor);
      }
      break;

    case BinInputTypeBuffer:
      //
      // Nothing to do - caller owns the buffer
      //
      break;

    default:
      break;
  }

  //
  // Clear structure
  //
  memset (Input, 0, sizeof (BINFORMAT_INPUT));
  Input->FileDescriptor = -1;
}
