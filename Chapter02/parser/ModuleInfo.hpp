#ifndef MODULEINFO_HPP
#define MODULEINFO_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

enum class SignatureType : uint8_t { I32 = 'i', I64 = 'I', F32 = 'f', F64 = 'F', PARAMSTART = '(', PARAMEND = ')' };

enum class WasmType : uint8_t {
  EXTERN_REF = 0x6F,
  FUNC_REF = 0x70,
  VEC_TYPE = 0x7B,
  F64 = 0x7C,
  F32 = 0x7D,
  I64 = 0x7E,
  I32 = 0x7F,
  TVOID = 0x40,
  INVALID = 0x00
};

enum class StorageType : uint8_t { STACKMEMORY, LINKDATA, REGISTER, CONSTANT, INVALID };

enum class aarch64REG : uint32_t { // clang-format off
  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, R16, R17, R18, R19, R20, R21, R22, R23, R24, R25, R26, R27, R28, FP, LR, ZR, SP = ZR,
  F0 = 0b0010'0000, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30, F31,
  NUMREGS,
  NONE = 0b1000'0000
}; // clang-format on

using TReg = aarch64REG;

class ModuleInfo final {
public:
  std::vector<std::string> types;

  class FunctionInfo final {
  public:
    uint32_t typeIndex = 0U;
    uint32_t numParams = 0U;
    uint32_t numLocals = 0U;
    uint32_t numLocalsInGPR = 0U;
    uint32_t numLocalsInFPR = 0U;
    uint32_t paramWidth = 0U;
    uint32_t directLocalsWidth = 0U;
    uint32_t stackFrameSize = 0U;

    bool unreachable = false;
    bool properlyTerminated = false;
  };

  class LocalVar final {
  public:
    WasmType wasmType = WasmType::INVALID;
    StorageType currentStorageType = StorageType::INVALID;

    TReg reg{};                       ///<  CPU register this variable is stored in (Index defined by the backend, if type is REGISTER)
    uint32_t stackFramePosition = 0U; ///< Offset in the current stack frame (if type is STACKMEMORY)
  };

  size_t functionNums;
  std::vector<uint32_t> functionTypeIndexs; // functionSection all function indexs

  std::vector<std::vector<LocalVar>> functionsLocalVars; // every index is func

  std::vector<std::vector<uint8_t>> functionsInstructions;
  // to do
  std::map<std::string, FunctionInfo> functions;
};

#endif
