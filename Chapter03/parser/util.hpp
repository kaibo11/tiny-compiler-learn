#include <cstdint>
#include <cstring>
#include <type_traits>
#include <string>

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
///
/// @brief Reinterprets a data type so that the underlying bit representation is unchanged. Unconditionally fulfills the
/// strict aliasing rule.
///
/// @tparam Dest Destination type
/// @tparam Source Source type
/// @param source Data to reinterpret
/// @return constexpr Dest Reinterpreted data
template <class Dest, class Source> constexpr Dest bit_cast(Source const &source) {
  static_assert(std::is_trivially_copyable<Source>::value, "bit_cast requires the source type to be copyable");
  static_assert(std::is_trivially_copyable<Dest>::value, "bit_cast requires the destination type to be copyable");
  static_assert(sizeof(Dest) == sizeof(Source), "bit_cast requires source and destination to be the same size");
  Dest dest;
  static_cast<void>(std::memcpy(&dest, &source, static_cast<size_t>(sizeof(dest))));
  return dest;
}

uint64_t convertStringToUint64(const std::string& str);

union ConstUnion {
  uint32_t u32; ///< 32-bit integer
  uint64_t u64; ///< 64-bit integer
  float f32;    ///< 32-bit float
  double f64;   ///< 64-bit float

  ///
  /// @brief Get the raw, reinterpreted value of the float as an integer
  ///
  /// @return uint32_t Raw, reinterpreted value of the float
  inline uint32_t rawF32() const {
    return bit_cast<uint32_t>(f32);
  }

  ///
  /// @brief Get the raw, reinterpreted value of the float as an integer
  ///
  /// @return uint64_t Raw, reinterpreted value of the float
  inline uint64_t rawF64() const {
    return bit_cast<uint64_t>(f64);
  }
};