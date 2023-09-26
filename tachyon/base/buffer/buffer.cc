#include "tachyon/base/buffer/buffer.h"

#include <string.h>

#include "tachyon/base/numerics/checked_math.h"

namespace tachyon::base {

bool Buffer::ReadAt(size_t buffer_offset, uint8_t* ptr, size_t size) const {
  base::CheckedNumeric<size_t> len = buffer_offset;
  size_t size_needed;
  if (!(len + size).AssignIfValid(&size_needed)) return false;
  if (size_needed > buffer_len_) {
    return false;
  }
  memcpy(ptr, reinterpret_cast<const char*>(buffer_) + buffer_offset, size);
  buffer_offset_ = buffer_offset + size;
  return true;
}

#define READ_BE_AT(bytes, bits, type)                                    \
  bool Buffer::Read##bits##BEAt(size_t buffer_offset, type* ptr) const { \
    base::CheckedNumeric<size_t> len = buffer_offset;                    \
    size_t size_needed;                                                  \
    if (!(len + bytes).AssignIfValid(&size_needed)) return false;        \
    if (size_needed > buffer_len_) {                                     \
      return false;                                                      \
    }                                                                    \
    type value = absl::big_endian::Load##bits(                           \
        &reinterpret_cast<char*>(buffer_)[buffer_offset]);               \
    memcpy(ptr, &value, bytes);                                          \
    buffer_offset_ = buffer_offset + bytes;                              \
    return true;                                                         \
  }

READ_BE_AT(2, 16, uint16_t)
READ_BE_AT(4, 32, uint32_t)
READ_BE_AT(8, 64, uint64_t)

#undef READ_BE_AT

#define READ_LE_AT(bytes, bits, type)                                    \
  bool Buffer::Read##bits##LEAt(size_t buffer_offset, type* ptr) const { \
    base::CheckedNumeric<size_t> len = buffer_offset;                    \
    size_t size_needed;                                                  \
    if (!(len + bytes).AssignIfValid(&size_needed)) return false;        \
    if (size_needed > buffer_len_) {                                     \
      return false;                                                      \
    }                                                                    \
    type value = absl::little_endian::Load##bits(                        \
        &reinterpret_cast<char*>(buffer_)[buffer_offset]);               \
    memcpy(ptr, &value, bytes);                                          \
    buffer_offset_ = buffer_offset + bytes;                              \
    return true;                                                         \
  }

READ_LE_AT(2, 16, uint16_t)
READ_LE_AT(4, 32, uint32_t)
READ_LE_AT(8, 64, uint64_t)

#undef READ_LE_AT

bool Buffer::WriteAt(size_t buffer_offset, const uint8_t* ptr, size_t size) {
  base::CheckedNumeric<size_t> len = buffer_offset;
  size_t size_needed;
  if (!(len + size).AssignIfValid(&size_needed)) return false;
  if (size_needed > buffer_len_) {
    if (!Grow(size_needed)) return false;
  }
  memcpy(reinterpret_cast<char*>(buffer_) + buffer_offset, ptr, size);
  buffer_offset_ = buffer_offset + size;
  return true;
}

#define WRITE_BE_AT(bytes, bits, type)                               \
  bool Buffer::Write##bits##BEAt(size_t buffer_offset, type value) { \
    base::CheckedNumeric<size_t> len = buffer_offset;                \
    size_t size_needed;                                              \
    if (!(len + bytes).AssignIfValid(&size_needed)) return false;    \
    if (size_needed > buffer_len_) {                                 \
      if (!Grow(size_needed)) return false;                          \
    }                                                                \
    absl::big_endian::Store##bits(                                   \
        &reinterpret_cast<char*>(buffer_)[buffer_offset], value);    \
    buffer_offset_ = buffer_offset + bytes;                          \
    return true;                                                     \
  }

WRITE_BE_AT(2, 16, uint16_t)
WRITE_BE_AT(4, 32, uint32_t)
WRITE_BE_AT(8, 64, uint64_t)

#undef WRITE_BE_AT

#define WRITE_LE_AT(bytes, bits, type)                               \
  bool Buffer::Write##bits##LEAt(size_t buffer_offset, type value) { \
    base::CheckedNumeric<size_t> len = buffer_offset;                \
    size_t size_needed;                                              \
    if (!(len + bytes).AssignIfValid(&size_needed)) return false;    \
    if (size_needed > buffer_len_) {                                 \
      if (!Grow(size_needed)) return false;                          \
    }                                                                \
    absl::little_endian::Store##bits(                                \
        &reinterpret_cast<char*>(buffer_)[buffer_offset], value);    \
    buffer_offset_ = buffer_offset + bytes;                          \
    return true;                                                     \
  }

WRITE_LE_AT(2, 16, uint16_t)
WRITE_LE_AT(4, 32, uint32_t)
WRITE_LE_AT(8, 64, uint64_t)

#undef WRITE_LE_AT

}  // namespace tachyon::base
