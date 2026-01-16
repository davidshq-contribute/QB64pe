//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File Lock Module
//  File locking and unlocking functions for multi-process file access control
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_FILE_LOCK_H
#define INCLUDE_LIBQB_FILE_LOCK_H

#include "../../os.h"  // For int32, int64, uint32, uint64 type definitions

// ============================================================================
// FILE LOCKING
// ============================================================================

/// Locks a portion of a file for exclusive access.
/// Prevents other processes from accessing the specified file region.
/// For sequential files (INPUT/OUTPUT), locks the entire file regardless of range.
/// @param i File number to lock
/// @param start Starting byte position (1-based, or -1 for entire file)
/// @param end Ending byte position (1-based, or -1 for entire file)
/// @param passed Bit flags: bit 0 = start passed, bit 1 = end passed
void sub_lock(int32 i, int64 start, int64 end, int32 passed);

/// Unlocks a portion of a file.
/// Releases a previously acquired lock on the specified file region.
/// For sequential files (INPUT/OUTPUT), unlocks the entire file regardless of range.
/// @param i File number to unlock
/// @param start Starting byte position (1-based, or -1 for entire file)
/// @param end Ending byte position (1-based, or -1 for entire file)
/// @param passed Bit flags: bit 0 = start passed, bit 1 = end passed
void sub_unlock(int32 i, int64 start, int64 end, int32 passed);

#endif // INCLUDE_LIBQB_FILE_LOCK_H
