//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File Lock Module
//  File locking and unlocking functions for multi-process file access control
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "file_lock.h"
#include "error_handle.h"
#include "gfs.h"

#include "../../os.h"  // For int32, int64, uint32, uint64 type definitions

// ============================================================================
// FILE LOCKING
// ============================================================================

void sub_lock(int32 i, int64 start, int64 end, int32 passed) {
    if (is_error_pending())
        return;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);

    // If the file has been opened for sequential input or output, LOCK and UNLOCK affect the entire file, regardless of the range specified by start& and end&.
    if (gfs->type > 2)
        passed = 0;

    if (passed & 1) {
        start--;
        if (start < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            start *= gfs->record_length;
    } else {
        start = -1;
    }

    if (passed & 2) {
        end--;
        if (end < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            end = end * gfs->record_length + gfs->record_length - 1;
    } else {
        end = start;
        if (gfs->type == 1)
            end = start + gfs->record_length - 1;
        if (!(passed & 1))
            end = -1;
    }

    int32 e;
    e = gfs_lock(i, start, end);
    if (e) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        if (e == -7) {
            error(70);
            return;
        } // permission denied
        error(75);
        return; // assume[-9]: path/file access error
    }
}

void sub_unlock(int32 i, int64 start, int64 end, int32 passed) {
    if (is_error_pending())
        return;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);

    // If the file has been opened for sequential input or output, LOCK and UNLOCK affect the entire file, regardless of the range specified by start& and end&.
    if (gfs->type > 2)
        passed = 0;

    if (passed & 1) {
        start--;
        if (start < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            start *= gfs->record_length;
    } else {
        start = -1;
    }

    if (passed & 2) {
        end--;
        if (end < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            end = end * gfs->record_length + gfs->record_length - 1;
    } else {
        end = start;
        if (gfs->type == 1)
            end = start + gfs->record_length - 1;
        if (!(passed & 1))
            end = -1;
    }

    int32 e;
    e = gfs_unlock(i, start, end);
    if (e) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        if (e == -7) {
            error(70);
            return;
        } // permission denied
        error(75);
        return; // assume[-9]: path/file access error
    }
}
