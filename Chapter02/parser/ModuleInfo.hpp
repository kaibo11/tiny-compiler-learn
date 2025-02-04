#ifndef MODULEINFO_HPP
#define MODULEINFO_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "aarch64_common.hpp"

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

class ModuleInfo final {
public:
  std::vector<std::string> signatureTypes;

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

  class FuncParm final {
  public:
    WasmType wasmType = WasmType::INVALID;
    StorageType currentStorageType = StorageType::INVALID;

    TReg reg{};                       ///<  CPU register this variable is stored in (Index defined by the backend, if type is REGISTER)
    uint32_t stackFramePosition = 0U; ///< Offset in the current stack frame (if type is STACKMEMORY)
  };

  size_t functionNums = 0;

  // every index is func
  std::vector<std::vector<LocalVar>> functionsLocalVars;
  std::vector<std::vector<FuncParm>> functionsParams;
  std::vector<std::vector<uint8_t>> functionsInstructions;
  std::vector<FunctionInfo> functionInfos;
  // every index is func end

  std::vector<std::vector<uint8_t>> machineCodes;

  // 函数名称到函数索引
  std::map<size_t, std::string> functionsIndexName;
  std::map<std::string, size_t> functionsNameIndex;
};

#endif
