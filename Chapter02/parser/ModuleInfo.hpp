#ifndef MODULEINFO_HPP
#define MODULEINFO_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

enum class SignatureType : uint8_t { I32 = 'i', I64 = 'I', F32 = 'f', F64 = 'F', PARAMSTART = '(', PARAMEND = ')' };

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

  std::map<std::string, FunctionInfo> functions;
  FunctionInfo fnc; ///< FunctionInfo instance for the current function
};

#endif
