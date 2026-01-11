// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "file.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>

#include <windows.h>
#undef min
#undef max

/**
 * @file mingw/file.cpp
 * @brief File I/O implementation for PE file parsing on Windows
 * 
 * This file implements buffered file I/O for reading PE format files.
 * Provides random access with efficient buffering.
 * 
 * Copyright Edd Dawson 2012, distributed under Boost Software License.
 */

/**
 * @brief Constructs a file handle
 * @param filename UTF-16 filename
 * @note Opens the file for reading, gets file size, and initializes buffer.
 *       Throws file_error on failure.
 */
file::file(const wchar_t *filename) :
    h(CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)),
    sz(0),
    next(buffer),
    buffer_begin_pos(0)
{
    if (!h || h == INVALID_HANDLE_VALUE)
        throw file_error("failed to open file");

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(static_cast<HANDLE>(h), &file_size))
    {
        CloseHandle(static_cast<HANDLE>(h));
        throw file_error("failed to get file size");
    }
    sz = file_size.QuadPart;

    refill(0);
}

file::~file() {
    CloseHandle(static_cast<HANDLE>(h));
}


/**
 * @brief Gets the file size
 * @return File size in bytes
 */
uint64_t file::size() const {
    return sz;
}

/**
 * @brief Seeks to a position in the file
 * @param pos Position to seek to
 * @note If position is in current buffer, adjusts buffer pointer.
 *       Otherwise, seeks file and refills buffer. Throws file_error on invalid position.
 */
void file::go(uint64_t pos) {
    if (pos > sz)
        throw file_error("bad stream cursor position specified");

    if (position_in_buffer(pos))
    {
        next += (pos - offset());
    }
    else
    {
        LARGE_INTEGER file_pos;
        file_pos.QuadPart = pos;

        LARGE_INTEGER new_cursor;

        if (!SetFilePointerEx(static_cast<HANDLE>(h), file_pos, &new_cursor, FILE_BEGIN))
            throw file_error("failed to move stream cursor");

        refill(pos);
    }
}

/**
 * @brief Skips forward or backward in the file
 * @param delta Number of bytes to skip (positive or negative)
 * @note Validates that skip doesn't go beyond file boundaries.
 *       Throws file_error on invalid position.
 */
void file::skip(int64_t delta) {
    if (delta == 0)
        return;

    const uint64_t cursor = offset();

    if ((delta < 0 && static_cast<uint64_t>(-delta) > cursor) ||
        (delta > 0 && static_cast<uint64_t>(delta) + cursor > sz))
    {
        throw file_error("bad stream cursor position specified");
    }

    go(cursor + delta);
}

/**
 * @brief Gets the current file offset
 * @return Current offset in bytes
 */
uint64_t file::offset() const {
    return buffer_begin_pos + (next - buffer);
}

/**
 * @brief Reads a 64-bit unsigned integer
 * @return 64-bit value
 */
uint64_t file::u64() {
    return read_integer<uint64_t>();
}

/**
 * @brief Reads a 32-bit unsigned integer
 * @return 32-bit value
 */
uint32_t file::u32() {
    return read_integer<uint32_t>();
}

/**
 * @brief Reads a 16-bit unsigned integer
 * @return 16-bit value
 */
uint16_t file::u16() {
    return read_integer<uint16_t>();
}

/**
 * @brief Reads an 8-bit unsigned integer
 * @return 8-bit value
 */
uint8_t file::u8() {
    return read_integer<uint8_t>();
}

/**
 * @brief Reads bytes from the file
 * @param bytes Buffer to read into
 * @param n Number of bytes to read
 * @note Handles buffering automatically. Reads from buffer if available,
 *       otherwise refills buffer. Throws file_error on read failure or EOF.
 */
void file::read(uint8_t *bytes, std::size_t n) {
    if (n + offset() > sz)
        throw file_error("failed to read from file");

    const uint8_t * const buffer_end = buffer + sizeof buffer;
    const uint8_t * const bytes_end = bytes + n;

    while (bytes != bytes_end)
    {
        const std::size_t in_buffer = 
            static_cast<std::size_t>(std::min<uint64_t>(buffer_end - next, sz - offset()));

        const std::size_t to_copy = std::min<std::size_t>(in_buffer, bytes_end - bytes);

        std::memcpy(bytes, next, to_copy);
        bytes += to_copy;
        next += to_copy;

        if (next == buffer_end)
            refill(buffer_begin_pos + sizeof buffer);
    }
}

/**
 * @brief Checks if a position is in the current buffer
 * @param pos Position to check
 * @return true if position is in buffer, false otherwise
 */
bool file::position_in_buffer(uint64_t pos) const {
    const uint64_t buffer_end_pos = std::min(buffer_begin_pos + sizeof buffer, sz);

    return pos >= buffer_begin_pos && pos < buffer_end_pos;
}

template<typename T>
T file::read_integer() {
    T ret = 0;
    read(reinterpret_cast<uint8_t *>(&ret), sizeof ret);
    return ret;
}

/**
 * @brief Refills the read buffer
 * @param new_begin_pos New buffer start position
 * @note Reads a new buffer full of data from the file starting at new_begin_pos.
 *       Throws file_error on read failure.
 */
void file::refill(std::size_t new_begin_pos) {
    buffer_begin_pos = new_begin_pos;
    next = buffer;

    const std::size_t to_read =
        static_cast<std::size_t>(std::min<uint64_t>(sizeof buffer, sz - buffer_begin_pos));

    if (to_read == 0)
        return;

    assert(to_read <= DWORD(-1)); // due to sizeof buffer

    DWORD num_read = 0;
    if (!ReadFile(static_cast<HANDLE>(h), buffer, static_cast<DWORD>(to_read), &num_read, 0) ||
        num_read != to_read)
    {
        throw file_error("failed to read from file");
    }
}
