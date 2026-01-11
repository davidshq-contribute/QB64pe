
#include "libqb-common.h"

#include <stdlib.h>
#include <string.h>

#include "cmem.h"
#include "error_handle.h"
#include "qbs.h"

/**
 * @file qbs_cmem.cpp
 * @brief Implementation of conventional memory (cmem) string management for QB64-PE
 * 
 * This file implements string management in conventional memory (DBLOCK), used for
 * QB64 programs that need to store strings in specific memory regions.
 * 
 * FIXME: conventional memory should be consolidated into libqb source and headers
 */

/**
 * @brief Stack pointer for qbs strings in conventional memory
 */
extern uint32_t qbs_cmem_sp; //=256;

/**
 * @brief Stack pointer for conventional memory
 */
extern uint32_t cmem_sp;     //=65536;

/**
 * @brief List tracking strings in conventional memory
 * @note Used to track strings in 16-bit memory (DBLOCK)
 */
static intptr_t *qbs_cmem_list = (intptr_t *)malloc(65536 * sizeof(intptr_t));

/**
 * @brief Last index in cmem list array
 */
static uint32_t qbs_cmem_list_lasti = 65535;

/**
 * @brief Next index to use in cmem list
 */
static uint32_t qbs_cmem_list_nexti = 0;

/**
 * @brief Descriptor space for cmem strings
 * @note Enough for 64 strings before expansion
 */
static uint32_t qbs_cmem_descriptor_space = 256; // enough for 64 strings before expansion

/**
 * @brief Removes a string from conventional memory tracking
 * @param str String to remove from cmem list
 * @note Does not release the actual descriptor, simply removes the string from the 'cmem' list.
 *       Assumes the string is in fact in cmem.
 */
void qbs_remove_cmem(qbs *str) {
    qbs_cmem_list[str->listi] = -1;
    if ((qbs_cmem_list_nexti - 1) == str->listi)
        qbs_cmem_list_nexti--;
}

/**
 * @brief Compacts the cmem string list by removing gaps
 * @note Removes freed entries from the list and updates list indices.
 *       Expands the list array if it's more than half full.
 *       Generates error 509 if reallocation fails.
 */
void qbs_cmem_concat_list() {
    uint32_t i;
    uint32_t d;
    qbs *tqbs;
    d = 0;
    for (i = 0; i < qbs_cmem_list_nexti; i++) {
        if (qbs_cmem_list[i] != -1) {
            if (i != d) {
                tqbs = (qbs *)qbs_cmem_list[i];
                tqbs->listi = d;
                qbs_cmem_list[d] = (intptr_t)tqbs;
            }
            d++;
        }
    }
    qbs_cmem_list_nexti = d;
    // if string listings are taking up more than half of the list array double the list array's size
    if (qbs_cmem_list_nexti >= (qbs_cmem_list_lasti / 2)) {
        qbs_cmem_list_lasti *= 2;
        qbs_cmem_list = (intptr_t *)realloc(qbs_cmem_list, (qbs_cmem_list_lasti + 1) * sizeof(*qbs_cmem_list));
        if (!qbs_cmem_list)
            error(509);
    }
    return;
}

/**
 * @brief Compacts cmem string data
 * @param bytesrequired Minimum number of bytes needed after compaction
 * @note As the cmem stack has a limit, if bytesrequired cannot be met this exits and returns an error.
 *       The cmem stack cannot be extended! So bytesrequired is only passed to possibly generate
 *       an error, or not generate one. Compacts strings by moving them closer together.
 *       Generates error 513 if insufficient space is available.
 */
void qbs_concat_cmem(uint32_t bytesrequired) {
    // this does not change indexing, only ->chr pointers and the location of their data
    uint32_t i;
    uint8_t *dest;
    qbs *tqbs;
    dest = (uint8_t *)dblock;
    qbs_cmem_sp = qbs_cmem_descriptor_space;
    if (qbs_cmem_list_nexti) {
        for (i = 0; i < qbs_cmem_list_nexti; i++) {
            if (qbs_cmem_list[i] != -1) {
                tqbs = (qbs *)qbs_cmem_list[i];
                if (tqbs->chr != dest) {
                    if (tqbs->len) {
                        memmove(dest, tqbs->chr, tqbs->len);
                    }
                    tqbs->chr = dest;
                    // update cmem_descriptor [length][offset]
                    if (tqbs->cmem_descriptor) {
                        tqbs->cmem_descriptor[0] = tqbs->len;
                        tqbs->cmem_descriptor[1] = (uint16_t)(intptr_t)(tqbs->chr - dblock);
                    }
                }
                dest += tqbs->len;
                qbs_cmem_sp += tqbs->len;
            }
        }
    }
    if ((qbs_cmem_sp + bytesrequired) > cmem_sp)
        error(513);
    return;
}

/**
 * @brief Creates a new string in conventional memory
 * @param size Size of the string to create
 * @param tmp Temporary flag
 * @param newstr Output string descriptor
 * @note Allocates string storage from the cmem stack. For non-temporary strings,
 *       allocates a descriptor in DBLOCK. Generates error 514 if descriptor space
 *       is exhausted. Generates error 513 if string space is exhausted.
 */
void qbs_create_cmem(int32_t size, uint8_t tmp, qbs *newstr) {
    if ((qbs_cmem_sp + size) > cmem_sp)
        qbs_concat_cmem(size);

    newstr->len = size;
    if ((qbs_cmem_sp + size) > cmem_sp)
        qbs_concat_cmem(size);
    newstr->chr = (uint8_t *)dblock + qbs_cmem_sp;
    qbs_cmem_sp += size;
    newstr->in_cmem = 1;
    if (qbs_cmem_list_nexti > qbs_cmem_list_lasti)
        qbs_cmem_concat_list();
    newstr->listi = qbs_cmem_list_nexti;
    qbs_cmem_list[newstr->listi] = (intptr_t)newstr;
    qbs_cmem_list_nexti++;
    if (tmp) {
        newstr->tmplisti = qbs_tmp_list_nexti;
        qbs_tmp_list[newstr->tmplisti] = (intptr_t)newstr;
        qbs_tmp_list_nexti++;
        newstr->tmp = 1;
    } else {
        // alloc string descriptor in DBLOCK (4 bytes)
        cmem_sp -= 4;
        newstr->cmem_descriptor = (uint16_t *)(dblock + cmem_sp);
        if (cmem_sp < qbs_cmem_sp)
            error(514);
        newstr->cmem_descriptor_offset = cmem_sp;
        // update cmem_descriptor [length][offset]
        newstr->cmem_descriptor[0] = newstr->len;
        newstr->cmem_descriptor[1] = (uint16_t)(intptr_t)(newstr->chr - dblock);
    }
}

/**
 * @brief Attempts to create a fixed string in cmem at the given offset
 * @param offset Memory offset for the string
 * @param size Size of the string
 * @param tmp Temporary flag (unused)
 * @param newstr Output string descriptor
 * @return true if successful (offset is in DBLOCK), false otherwise
 * @note Checks if the offset is within DBLOCK range. If so, allocates a descriptor
 *       and sets up the string. Generates error 515 if descriptor space is exhausted.
 */
bool qbs_new_fixed_cmem(uint8_t *offset, uint32_t size, uint8_t tmp, qbs *newstr) {
    (void)size;
    (void)tmp;

    // is it in DBLOCK?
    if ((offset > (cmem + 1280)) && (offset < (cmem + 66816))) {
        // alloc string descriptor in DBLOCK (4 bytes)
        cmem_sp -= 4;
        newstr->cmem_descriptor = (uint16_t *)(dblock + cmem_sp);
        if (cmem_sp < qbs_cmem_sp)
            error(515);
        newstr->cmem_descriptor_offset = cmem_sp;
        // update cmem_descriptor [length][offset]
        newstr->cmem_descriptor[0] = newstr->len;
        newstr->cmem_descriptor[1] = (uint16_t)(intptr_t)(newstr->chr - dblock);

        return true;
    }

    return false;
}

/**
 * @brief Moves a string from source to destination in conventional memory
 * @param deststr Destination string
 * @param srcstr Source string
 * @note Assumes you've already checked that this is valid - both strings should be
 *       in cmem list, and srcstr should be acquirable by deststr with no copy.
 *       Does not do all the work, the return string move for non-cmem still needs to happen.
 *       Updates list indices and cmem descriptors.
 */
void qbs_move_cmem(qbs *deststr, qbs *srcstr) {
    // unlist deststr and acquire srcstr's list index
    qbs_cmem_list[deststr->listi] = -1;
    qbs_cmem_list[srcstr->listi] = (intptr_t)deststr;
    deststr->listi = srcstr->listi;

    if (deststr->cmem_descriptor) {
        deststr->cmem_descriptor[0] = srcstr->len;
        deststr->cmem_descriptor[1] = (uint16_t)(intptr_t)(srcstr->chr - dblock);
    }
}

/**
 * @brief Copies a string in conventional memory
 * @param deststr Destination string
 * @param srcstr Source string
 * @note Attempts to copy in-place if space is available. Otherwise compacts cmem
 *       and copies. Updates cmem descriptors if present.
 */
void qbs_copy_cmem(qbs *deststr, qbs *srcstr) {
    uint32_t i;
    qbs *tqbs;

    if (deststr->listi == (qbs_cmem_list_nexti - 1)) {                                // last index
        if (((intptr_t)deststr->chr + srcstr->len) <= (dblock + (intptr_t)cmem_sp)) { // space available
            memcpy(deststr->chr, srcstr->chr, srcstr->len);
            deststr->len = srcstr->len;
            qbs_cmem_sp = ((intptr_t)deststr->chr) + (intptr_t)deststr->len - dblock;
            goto update_cmem_descriptor;
        }
        goto qbs_set_cmem_concat_required;
    }
    // deststr is not the last index so locate next valid index
    i = deststr->listi + 1;
qbs_set_nextindex:
    if (qbs_cmem_list[i] != -1) {
        tqbs = (qbs *)qbs_cmem_list[i];
        if (tqbs == srcstr) {
            if (srcstr->tmp == 1)
                goto skippedtmpsrcindex;
        }
        if ((deststr->chr + srcstr->len) > tqbs->chr)
            goto qbs_set_cmem_concat_required;
        memcpy(deststr->chr, srcstr->chr, srcstr->len);
        deststr->len = srcstr->len;
        goto update_cmem_descriptor;
    }
skippedtmpsrcindex:
    i++;
    if (i != qbs_cmem_list_nexti)
        goto qbs_set_nextindex;
    // all next indexes invalid!
    qbs_cmem_list_nexti = deststr->listi + 1;                                     // adjust nexti
    if (((intptr_t)deststr->chr + srcstr->len) <= (dblock + (intptr_t)cmem_sp)) { // space available
        memmove(deststr->chr, srcstr->chr, srcstr->len);                          // overlap possible due to sometimes acquiring srcstr's space
        deststr->len = srcstr->len;
        qbs_cmem_sp = ((intptr_t)deststr->chr) + (intptr_t)deststr->len - dblock;
        goto update_cmem_descriptor;
    }
qbs_set_cmem_concat_required:
    // srcstr could not fit in deststr
    //"realloc" deststr
    qbs_cmem_list[deststr->listi] = -1;          // unlist
    if ((qbs_cmem_sp + srcstr->len) > cmem_sp) { // must concat!
        qbs_concat_cmem(srcstr->len);
    }
    if (qbs_cmem_list_nexti > qbs_cmem_list_lasti)
        qbs_cmem_concat_list();
    deststr->listi = qbs_cmem_list_nexti;
    qbs_cmem_list[qbs_cmem_list_nexti] = (intptr_t)deststr;
    qbs_cmem_list_nexti++; // relist
    deststr->chr = (uint8_t *)dblock + qbs_cmem_sp;
    deststr->len = srcstr->len;
    qbs_cmem_sp += deststr->len;
    memcpy(deststr->chr, srcstr->chr, srcstr->len);

update_cmem_descriptor:
    // update cmem_descriptor [length][offset]
    if (deststr->cmem_descriptor) {
        deststr->cmem_descriptor[0] = deststr->len;
        deststr->cmem_descriptor[1] = (uint16_t)(intptr_t)(deststr->chr - dblock);
    }
}
