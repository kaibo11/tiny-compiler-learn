///
/// @file StackElement.hpp
/// @copyright Copyright (C) 2021 BMW Group
///

#ifndef STACKELEMENT_HPP
#define STACKELEMENT_HPP

#include <cstdint>
#include <type_traits>

#include "ModuleInfo.hpp"
#include "OPCode.hpp"
#include "aarch64_common.hpp"
#include "util.hpp"

class StackType final {
public:
  constexpr inline StackType() : StackType(0U) {
  }

  ///
  /// @param raw The raw uint32_t value of this stack type.
  constexpr inline explicit StackType(uint32_t const raw) : raw_(raw) {
  }

  template <typename RHS> inline constexpr StackType &operator=(RHS const rhs) {
    static_assert(std::is_same<RHS, StackType>::value || std::is_same<RHS, uint32_t>::value, "RHS must be either StackType or uint32_t");
    raw_ = static_cast<uint32_t>(rhs);
    return *this;
  }

  static constexpr uint32_t INVALID{0U};     ///< Invalid StackElement, not representing any actual operand
  static constexpr uint32_t SANULL{INVALID}; ///< StackElement with undefined Type

  static constexpr uint32_t SCRATCHREGISTER{1U}; ///< StackElement representing a variable in a scratch register
  static constexpr uint32_t TEMPSTACK{2U};       ///< StackElement representing a variable on the stack (not a param or global)
  static constexpr uint32_t CONSTANT{3U};        ///< StackElement representing a constant

  static constexpr uint32_t LOCAL{4U};  ///< StackElement representing a local variable (Can be on stack or in a register, actual location defined in
                                        ///< the corresponding LocalDef)
  static constexpr uint32_t GLOBAL{5U}; ///< StackElement representing a global variable (Actual location defined in the corresponding GlobalDef)

  static constexpr uint32_t DEFERREDACTION{6U}; ///< StackElement representing a deferred action, i.e. an arithmetic instruction, conversion etc. that
                                                ///< has not been emitted yet

  static constexpr uint32_t BLOCK{7U};   ///< StackElement representing the opening of a structural block
  static constexpr uint32_t LOOP{8U};    ///< StackElement representing the opening of a structural loop
  static constexpr uint32_t IFBLOCK{9U}; ///< StackElement representing a synthetic block that is inserted to properly realize branches for IF
                                         ///< statements

  static constexpr uint32_t SKIP{10U}; ///< StackElements that will be skipped when traversing; inserted when iteratively condensing valent blocks

  // Flags for scratchRegister, tempStack and constant

  static constexpr uint32_t TVOID{0b0000'0000U};                        ///< void
  static constexpr uint32_t I32{0b0001'0000U};                          ///< int32
  static constexpr uint32_t SCRATCHREGISTER_I32{SCRATCHREGISTER | I32}; ///< int32 in scratch register
  static constexpr uint32_t TEMPSTACK_I32{TEMPSTACK | I32};             ///< int32 in tmp stack
  static constexpr uint32_t CONSTANT_I32{CONSTANT | I32};               ///< int32 const

  static constexpr uint32_t I64{0b0010'0000U};                          ///< int64
  static constexpr uint32_t SCRATCHREGISTER_I64{SCRATCHREGISTER | I64}; ///< int64 in scratch register
  static constexpr uint32_t TEMPSTACK_I64{TEMPSTACK | I64};             ///< int64 in tmp stack
  static constexpr uint32_t CONSTANT_I64{CONSTANT | I64};               ///< int64 const

  static constexpr uint32_t F32{0b0100'0000U};                          ///< float32
  static constexpr uint32_t SCRATCHREGISTER_F32{SCRATCHREGISTER | F32}; ///< float32 in scratch register
  static constexpr uint32_t TEMPSTACK_F32{TEMPSTACK | F32};             ///< float32 in tmp stack
  static constexpr uint32_t CONSTANT_F32{CONSTANT | F32};               ///< float32 const

  static constexpr uint32_t F64{0b1000'0000U};                          ///< float64
  static constexpr uint32_t SCRATCHREGISTER_F64{SCRATCHREGISTER | F64}; ///< float64 in scratch register
  static constexpr uint32_t TEMPSTACK_F64{TEMPSTACK | F64};             ///< float64 in tmp stack
  static constexpr uint32_t CONSTANT_F64{CONSTANT | F64};               ///< float64 const
  static constexpr uint32_t UNKNOWN{0b1111'0000U};                      ///< unknown
  static constexpr uint32_t BASEMASK{0b0000'1111U};                     ///< mask of base type
  static constexpr uint32_t TYPEMASK{0b1111'0000U};                     ///< mask of type(i32/i64/f32/f64)

  template <typename RHS> inline constexpr StackType operator&(RHS const rhs) const {
    return StackType{this->raw_ & static_cast<uint32_t>(rhs)};
  }

  explicit inline constexpr operator uint32_t() const {
    return raw_;
  }

private:
  uint32_t raw_;
};

class StackElement final {
public:
  StackType type;

  union Data {
    OPCode opcode;
    ConstUnion constUnion{};
  };

  class VariableData final {
  public:
    struct Location {
      uint32_t localIdx;           ///< Index of this local variable (if type is LOCAL)
      uint32_t globalIdx;          ///< Index of this global variable (if type is GLOBAL)
      TReg reg;                    ///<  CPU register where this temporary variable is stored (Index defined by the backend, if type is
                                   ///<  SCRATCHREGISTER)
      uint32_t stackFramePosition; ///< Offset in the current function stack frame (if type is TEMPSTACK)

      WasmType wasmtype;
    };
    Location location; ///< Location where this variable is stored

    /// @brief Linked list to quickly iterate copies of variables on the stack (e.g. when spilling variables)
    struct IndexData final {
      uint32_t prevOccurrence;     ///< Index on the stack of the previous occurrence/copy (not necessarily in order on
                                   ///< the stack)
      uint32_t nextOccurrence;     ///< Index on the stack of the next occurrence/copy (not necessarily in order on the stack)
      uint32_t nextLowerTempStack; ///< Index of the temporary stack variable with the next lower stack offset (only
                                   ///< active for TEMPSTACK elements)
    };
    IndexData indexData = {}; ///< Data enabling linked-list traversals of copies of variables on the stack
  };

  Data data = {};

  VariableData variableData;

  static inline constexpr StackElement i32Const(uint32_t const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_I32;
    res.data.constUnion.u32 = value;
    return res;
  }

  static inline constexpr StackElement i64Const(uint64_t const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_I64;
    res.data.constUnion.u64 = value;
    return res;
  }

  static inline constexpr StackElement f32Const(float const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_F32;
    res.data.constUnion.f32 = value;
    return res;
  }

  static inline constexpr StackElement f64Const(double const value) {
    StackElement res{};
    res.type = StackType::CONSTANT_F64;
    res.data.constUnion.f64 = value;
    return res;
  }

  // static inline constexpr StackElement local(uint32_t const localIdx) {
  //   StackElement res{};
  //   res.type = StackType::LOCAL;
  //   res.data.variableData.location.localIdx = localIdx;
  //   return res;
  // }

  // static inline constexpr StackElement global(uint32_t const globalIdx) VB_NOEXCEPT {
  //   StackElement res{};
  //   res.type = StackType::GLOBAL;
  //   res.data.variableData.location.globalIdx = globalIdx;
  //   return res;
  // }

  // static inline constexpr StackElement skip(uint32_t const skipCount) {
  //   StackElement res{};
  //   res.type = StackType::SKIP;
  //   res.data.skipCount = skipCount;
  //   return res;
  // }

  // static inline constexpr StackElement invalid() {
  //   StackElement res{};
  //   res.type = StackType::INVALID;
  //   return res;
  // }

  // static inline constexpr StackElement action(OPCode const instruction) VB_NOEXCEPT {
  //   StackElement res{};
  //   res.type = StackType::DEFERREDACTION;
  //   res.data.opcode = instruction;
  //   return res;
  // }
};

#endif
