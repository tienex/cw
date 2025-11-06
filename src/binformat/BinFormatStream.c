/** @file
  Binary Format Stream I/O Implementation.

  This file implements the unified stream abstraction layer that allows
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
  Initialize stream from file path.

  @param[out]  Stream            Pointer to stream structure.
  @param[in]   FilePath          Path to file.
  @param[in]   Flags             Stream flags (BINFORMAT_STREAM_*).

  @retval BINFORMAT_SUCCESS      Stream initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamInitFile(
  OUT BINFORMAT_STREAM  *Stream,
  IN  CONST CHAR8       *FilePath,
  IN  UINT32            Flags
  )
{
  FILE           *File;
  UINT64         FileSize;
  UINT8          *Buffer;
  size_t         BytesRead;
  INT32          Fd;
  struct stat    St;
  VOID           *MappedData;
  INT32          Prot;
  INT32          MmapFlags;
  BOOLEAN        IsReadOnly;

  if (Stream == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  memset (Stream, 0, sizeof (BINFORMAT_STREAM));
  Stream->Flags = Flags;
  Stream->FilePath = strdup (FilePath);
  Stream->FileDescriptor = -1;

  IsReadOnly = !(Flags & BINFORMAT_STREAM_WRITE);

  //
  // Memory-mapped file
  //
  if (Flags & BINFORMAT_STREAM_MMAP) {
    Fd = open (FilePath, IsReadOnly ? O_RDONLY : O_RDWR);
    if (Fd < 0) {
      free (Stream->FilePath);
      return BINFORMAT_ERROR_IO;
    }

    if (fstat (Fd, &St) < 0) {
      close (Fd);
      free (Stream->FilePath);
      return BINFORMAT_ERROR_IO;
    }

    Prot      = IsReadOnly ? PROT_READ : (PROT_READ | PROT_WRITE);
    MmapFlags = IsReadOnly ? MAP_PRIVATE : MAP_SHARED;

    MappedData = mmap (NULL, (size_t)St.st_size, Prot, MmapFlags, Fd, 0);
    if (MappedData == MAP_FAILED) {
      close (Fd);
      free (Stream->FilePath);
      return BINFORMAT_ERROR_IO;
    }

    Stream->Data           = (UINT8 *)MappedData;
    Stream->Size           = (UINT64)St.st_size;
    Stream->Capacity       = (UINT64)St.st_size;
    Stream->FileDescriptor = Fd;
    Stream->MmapBase       = MappedData;

    return BINFORMAT_SUCCESS;
  }

  //
  // Streaming I/O
  //
  if (Flags & BINFORMAT_STREAM_STREAMING) {
    Fd = open (FilePath, IsReadOnly ? O_RDONLY : O_RDWR);
    if (Fd < 0) {
      free (Stream->FilePath);
      return BINFORMAT_ERROR_IO;
    }

    if (fstat (Fd, &St) < 0) {
      close (Fd);
      free (Stream->FilePath);
      return BINFORMAT_ERROR_IO;
    }

    Stream->Data           = NULL;
    Stream->Size           = (UINT64)St.st_size;
    Stream->Capacity       = 0;
    Stream->FileDescriptor = Fd;
    Stream->FilePosition   = 0;

    return BINFORMAT_SUCCESS;
  }

  //
  // Regular file I/O (load entire file into memory)
  //
  File = fopen (FilePath, IsReadOnly ? "rb" : "r+b");
  if (File == NULL) {
    free (Stream->FilePath);
    return BINFORMAT_ERROR_IO;
  }

  fseek (File, 0, SEEK_END);
  FileSize = (UINT64)ftell (File);
  fseek (File, 0, SEEK_SET);

  Buffer = (UINT8 *)malloc (FileSize);
  if (Buffer == NULL) {
    fclose (File);
    free (Stream->FilePath);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  BytesRead = fread (Buffer, 1, (size_t)FileSize, File);
  fclose (File);

  if (BytesRead != (size_t)FileSize) {
    free (Buffer);
    free (Stream->FilePath);
    return BINFORMAT_ERROR_IO;
  }

  Stream->Data     = Buffer;
  Stream->Size     = FileSize;
  Stream->Capacity = FileSize;

  return BINFORMAT_SUCCESS;
}

/**
  Initialize stream from existing buffer.

  @param[out]  Stream            Pointer to stream structure.
  @param[in]   Buffer            Pointer to buffer data.
  @param[in]   Size              Size of buffer.
  @param[in]   Flags             Stream flags (must include BINFORMAT_STREAM_BUFFER).

  @retval BINFORMAT_SUCCESS      Stream initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamInitBuffer(
  OUT BINFORMAT_STREAM  *Stream,
  IN  CONST VOID        *Buffer,
  IN  UINT64            Size,
  IN  UINT32            Flags
  )
{
  if (Stream == NULL || Buffer == NULL || Size == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (!(Flags & BINFORMAT_STREAM_BUFFER)) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  memset (Stream, 0, sizeof (BINFORMAT_STREAM));
  Stream->Flags          = Flags;
  Stream->FilePath       = NULL;
  Stream->Data           = (UINT8 *)Buffer;
  Stream->Size           = Size;
  Stream->Capacity       = Size;
  Stream->FileDescriptor = -1;

  return BINFORMAT_SUCCESS;
}

/**
  Initialize stream with allocated buffer (for creating new binary images).

  @param[out]  Stream            Pointer to stream structure.
  @param[in]   InitialSize       Initial buffer size (0 for default).

  @retval BINFORMAT_SUCCESS      Stream initialized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamInitAllocated(
  OUT BINFORMAT_STREAM  *Stream,
  IN  UINT64            InitialSize
  )
{
  UINT8  *Buffer;
  UINT64 AllocSize;

  if (Stream == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  AllocSize = (InitialSize == 0) ? DEFAULT_ALLOCATED_SIZE : InitialSize;

  Buffer = (UINT8 *)malloc ((size_t)AllocSize);
  if (Buffer == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  memset (Buffer, 0, (size_t)AllocSize);

  memset (Stream, 0, sizeof (BINFORMAT_STREAM));
  Stream->Flags          = BINFORMAT_STREAM_READ | BINFORMAT_STREAM_WRITE | BINFORMAT_STREAM_ALLOCATED;
  Stream->FilePath       = NULL;
  Stream->Data           = Buffer;
  Stream->Size           = 0;  // No data yet, just allocated space
  Stream->Capacity       = AllocSize;
  Stream->FileDescriptor = -1;

  return BINFORMAT_SUCCESS;
}

/**
  Read data from stream at specified offset.

  @param[in]  Stream             Stream structure.
  @param[in]  Offset             Offset to read from.
  @param[out] Buffer             Buffer to read into.
  @param[in]  Size               Number of bytes to read.

  @retval BINFORMAT_SUCCESS      Data read successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamRead(
  IN  BINFORMAT_STREAM  *Stream,
  IN  UINT64            Offset,
  OUT VOID              *Buffer,
  IN  UINT64            Size
  )
{
  ssize_t  BytesRead;

  if (Stream == NULL || Buffer == NULL || Size == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (Offset + Size > Stream->Size) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Streaming mode - read from file
  //
  if (Stream->Flags & BINFORMAT_STREAM_STREAMING) {
    if (lseek (Stream->FileDescriptor, (off_t)Offset, SEEK_SET) < 0) {
      return BINFORMAT_ERROR_IO;
    }

    BytesRead = read (Stream->FileDescriptor, Buffer, (size_t)Size);
    if (BytesRead != (ssize_t)Size) {
      return BINFORMAT_ERROR_IO;
    }

    Stream->FilePosition = Offset + Size;
    return BINFORMAT_SUCCESS;
  }

  //
  // Buffered mode - simple memory copy
  //
  if (Stream->Data != NULL) {
    memcpy (Buffer, Stream->Data + Offset, (size_t)Size);
    return BINFORMAT_SUCCESS;
  }

  return BINFORMAT_ERROR_INVALID_PARAMETER;
}

/**
  Write data to stream at specified offset.

  @param[in]  Stream             Stream structure.
  @param[in]  Offset             Offset to write to.
  @param[in]  Buffer             Buffer to write from.
  @param[in]  Size               Number of bytes to write.

  @retval BINFORMAT_SUCCESS      Data written successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamWrite(
  IN  BINFORMAT_STREAM  *Stream,
  IN  UINT64            Offset,
  IN  CONST VOID        *Buffer,
  IN  UINT64            Size
  )
{
  BINFORMAT_STATUS  Status;
  ssize_t           BytesWritten;

  if (Stream == NULL || Buffer == NULL || Size == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (!(Stream->Flags & BINFORMAT_STREAM_WRITE)) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Streaming mode - write to file
  //
  if (Stream->Flags & BINFORMAT_STREAM_STREAMING) {
    if (Offset + Size > Stream->Size) {
      Stream->Size = Offset + Size;
    }

    if (lseek (Stream->FileDescriptor, (off_t)Offset, SEEK_SET) < 0) {
      return BINFORMAT_ERROR_IO;
    }

    BytesWritten = write (Stream->FileDescriptor, Buffer, (size_t)Size);
    if (BytesWritten != (ssize_t)Size) {
      return BINFORMAT_ERROR_IO;
    }

    Stream->FilePosition = Offset + Size;
    return BINFORMAT_SUCCESS;
  }

  //
  // Allocated buffer - auto-resize if needed
  //
  if (Stream->Flags & BINFORMAT_STREAM_ALLOCATED) {
    if (Offset + Size > Stream->Capacity) {
      UINT64  NewCapacity;

      NewCapacity = Offset + Size;
      NewCapacity = (NewCapacity * GROWTH_FACTOR_NUM) / GROWTH_FACTOR_DEN;

      Status = BinFormatStreamResize (Stream, NewCapacity);
      if (BINFORMAT_IS_ERROR (Status)) {
        return Status;
      }
    }

    memcpy (Stream->Data + Offset, Buffer, (size_t)Size);

    if (Offset + Size > Stream->Size) {
      Stream->Size = Offset + Size;
    }

    return BINFORMAT_SUCCESS;
  }

  //
  // Other buffered types - check bounds
  //
  if (Stream->Data != NULL) {
    if (Offset + Size > Stream->Size) {
      return BINFORMAT_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy (Stream->Data + Offset, Buffer, (size_t)Size);
    return BINFORMAT_SUCCESS;
  }

  return BINFORMAT_ERROR_INVALID_PARAMETER;
}

/**
  Resize an allocated buffer.

  @param[in]  Stream             Stream structure.
  @param[in]  NewSize            New buffer size.

  @retval BINFORMAT_SUCCESS      Buffer resized successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamResize(
  IN  BINFORMAT_STREAM  *Stream,
  IN  UINT64            NewSize
  )
{
  UINT8  *NewBuffer;

  if (Stream == NULL || NewSize == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (!(Stream->Flags & BINFORMAT_STREAM_ALLOCATED)) {
    return BINFORMAT_ERROR_UNSUPPORTED;
  }

  if (NewSize <= Stream->Capacity) {
    if (NewSize > Stream->Size) {
      memset (Stream->Data + Stream->Size, 0, (size_t)(NewSize - Stream->Size));
    }
    Stream->Size = NewSize;
    return BINFORMAT_SUCCESS;
  }

  NewBuffer = (UINT8 *)realloc (Stream->Data, (size_t)NewSize);
  if (NewBuffer == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  if (NewSize > Stream->Size) {
    memset (NewBuffer + Stream->Size, 0, (size_t)(NewSize - Stream->Size));
  }

  Stream->Data     = NewBuffer;
  Stream->Size     = NewSize;
  Stream->Capacity = NewSize;

  return BINFORMAT_SUCCESS;
}

/**
  Flush any pending writes to disk.

  @param[in]  Stream             Stream structure.

  @retval BINFORMAT_SUCCESS      Data flushed successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatStreamFlush(
  IN  BINFORMAT_STREAM  *Stream
  )
{
  if (Stream == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (Stream->Flags & BINFORMAT_STREAM_STREAMING) {
    if (fsync (Stream->FileDescriptor) < 0) {
      return BINFORMAT_ERROR_IO;
    }
    return BINFORMAT_SUCCESS;
  }

  if (Stream->Flags & BINFORMAT_STREAM_MMAP) {
    if (!(Stream->Flags & BINFORMAT_STREAM_READ) || (Stream->Flags & BINFORMAT_STREAM_WRITE)) {
      if (msync (Stream->MmapBase, (size_t)Stream->Size, MS_SYNC) < 0) {
        return BINFORMAT_ERROR_IO;
      }
    }
    return BINFORMAT_SUCCESS;
  }

  return BINFORMAT_SUCCESS;
}

/**
  Get direct pointer to data (if available).

  @param[in]  Stream             Stream structure.

  @return Pointer to data, or NULL if not available.
**/
VOID *
BinFormatStreamGetDataPointer(
  IN  BINFORMAT_STREAM  *Stream
  )
{
  if (Stream == NULL) {
    return NULL;
  }

  if (Stream->Flags & BINFORMAT_STREAM_STREAMING) {
    return NULL;
  }

  return Stream->Data;
}

/**
  Close and free stream resources.

  @param[in]  Stream             Stream structure.
**/
VOID
BinFormatStreamClose(
  IN  BINFORMAT_STREAM  *Stream
  )
{
  if (Stream == NULL) {
    return;
  }

  if (Stream->FilePath != NULL) {
    free (Stream->FilePath);
    Stream->FilePath = NULL;
  }

  if (Stream->Flags & BINFORMAT_STREAM_MMAP) {
    if (Stream->MmapBase != NULL) {
      munmap (Stream->MmapBase, (size_t)Stream->Size);
      Stream->MmapBase = NULL;
    }
    if (Stream->FileDescriptor >= 0) {
      close (Stream->FileDescriptor);
      Stream->FileDescriptor = -1;
    }
  } else if (Stream->Flags & BINFORMAT_STREAM_STREAMING) {
    if (Stream->FileDescriptor >= 0) {
      close (Stream->FileDescriptor);
      Stream->FileDescriptor = -1;
    }
  } else if (Stream->Flags & BINFORMAT_STREAM_ALLOCATED) {
    if (Stream->Data != NULL) {
      free (Stream->Data);
      Stream->Data = NULL;
    }
  } else if (!(Stream->Flags & BINFORMAT_STREAM_BUFFER)) {
    //
    // Regular file mode - free the buffer we loaded
    //
    if (Stream->Data != NULL) {
      free (Stream->Data);
      Stream->Data = NULL;
    }
  }
  // For BINFORMAT_STREAM_BUFFER, caller owns the buffer

  memset (Stream, 0, sizeof (BINFORMAT_STREAM));
  Stream->FileDescriptor = -1;
}
