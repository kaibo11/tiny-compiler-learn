#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>

#include "ModuleInfo.hpp"
#include "OPCode.hpp"
#include "Stack.hpp"
#include "StackElement.hpp"
#include "aarch64_assembler.hpp"
#include "aarch64_common.hpp"
#include "parser.hpp"

uint32_t readULEB128(const std::vector<uint8_t> &data, size_t &index) {
  uint32_t result = 0;
  uint32_t shift = 0;
  const int maxBytes = 5; // ULEB128 for 32-bit integers should not exceed 5 bytes

  for (int byteCount = 0; byteCount < maxBytes; ++byteCount) {
    if (index >= data.size()) {
      throw std::out_of_range("ULEB128 encoding is incomplete or data is truncated.");
    }

    const uint8_t byte = data[index++];
    result |= (byte & 0x7FU) << shift;

    if ((byte & 0x80U) == 0) {
      // If the highest bit is not set, this is the last byte
      return result;
    }

    shift += 7;
  }

  throw std::overflow_error("ULEB128 encoding exceeds the maximum length for 32-bit integers.");
}

void parseTypeSection(const std::vector<uint8_t> &byteStream, size_t &index, ModuleInfo &moduleInfo) {
  uint32_t const sectionSize = readULEB128(byteStream, index);
  static_cast<void>(sectionSize);
  uint32_t numTypeSize = readULEB128(byteStream, index);
  while (numTypeSize-- > 0) {
    uint32_t const typeType = readULEB128(byteStream, index);
    if (typeType != 0x60) {
      std::cout << "parser not support non function type" << typeType << std::endl;
      exit(1);
    }
    uint32_t paraNums = readULEB128(byteStream, index);
    std::string funcSignatureType{static_cast<char>(SignatureType::PARAMSTART)};
    while (paraNums-- > 0) {
      switch (byteStream[index]) {
      case 0x7F: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::I32));
        break;
      }
      case 0x7E: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::I64));
        break;
      }
      case 0x7D: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::F32));
        break;
      }
      case 0x7C: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::F64));
        break;
      }
      default: {
        std::cout << "met unknown func SignatureType, exit.";
        exit(1);
      }
      }
      index++;
    }
    funcSignatureType.push_back(static_cast<char>(SignatureType::PARAMEND));
    uint32_t retNums = readULEB128(byteStream, index);
    if (retNums > 1) {
      std::cout << "wasm ret nums not support > 1. exit." << std::endl;
      exit(1);
    }
    while (retNums-- > 0) {
      switch (byteStream[index]) {
      case 0x7F: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::I32));
        break;
      }
      case 0x7E: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::I64));
        break;
      }
      case 0x7D: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::F32));
        break;
      }
      case 0x7C: {
        funcSignatureType.push_back(static_cast<char>(SignatureType::F64));
        break;
      }
      default: {
        std::cout << "met unknown func SignatureType, exit.";
        exit(1);
      }
      }
      index++;
    }
    std::cout << "get a funcSignatureType: " << funcSignatureType << std::endl;
    moduleInfo.signatureTypes.emplace_back(funcSignatureType);
  }
}

void parseExportSection(const std::vector<uint8_t> &byteStream, size_t &index, ModuleInfo &moduleInfo) {
  uint32_t const sectionSize = readULEB128(byteStream, index);
  static_cast<void>(sectionSize);
  uint32_t exportNums = readULEB128(byteStream, index);
  while (exportNums-- > 0) {
    size_t fieldNameSize = readULEB128(byteStream, index);
    std::string fieldName;
    char c;
    while (true) {
      c = static_cast<char>(byteStream[index++]);
      if (c == 0x00) { // get export name
        const size_t funcIndex = byteStream[index++];
        moduleInfo.functionsIndexName.emplace(std::make_pair(funcIndex, fieldName));
        moduleInfo.functionsNameIndex.emplace(std::make_pair(fieldName, funcIndex));
        break;
      }
      fieldName.push_back(c);
    }
  }
}

void parseFunctionSection(const std::vector<uint8_t> &byteStream, size_t &index, ModuleInfo &moduleInfo) {
  uint32_t const sectionSize = readULEB128(byteStream, index);
  static_cast<void>(sectionSize);
  uint32_t functionNums = readULEB128(byteStream, index);
  moduleInfo.functionNums = functionNums;
  std::cout << "get functionNums: " << functionNums << std::endl;
  while (functionNums-- > 0) {
    uint32_t const functionIndex = readULEB128(byteStream, index);
    moduleInfo.functionInfos.emplace_back(ModuleInfo::FunctionInfo{functionIndex});
  }
}

void parseCodeSection(const std::vector<uint8_t> &byteStream, size_t &index, ModuleInfo &moduleInfo) {
  uint32_t const sectionSize = readULEB128(byteStream, index);
  static_cast<void>(sectionSize);
  uint32_t functionSize = readULEB128(byteStream, index);
  while (functionSize-- > 0) {
    uint32_t const functionBodySize = readULEB128(byteStream, index);
    static_cast<void>(functionBodySize);
    uint32_t const localVarSizeIndex = index;
    uint32_t localVarSize = readULEB128(byteStream, index);
    std::vector<ModuleInfo::LocalVar> localVars;
    while (localVarSize-- > 0) {
      uint32_t localVarRepeatTimes = readULEB128(byteStream, index);
      uint8_t const localVarType = byteStream[index];
      ModuleInfo::LocalVar localVar;
      switch (localVarType) {
      case 0x7F: {
        localVar.wasmType = WasmType::I32;
        break;
      }
      case 0x7E: {
        localVar.wasmType = WasmType::I64;
        break;
      }
      case 0x7D: {
        localVar.wasmType = WasmType::F32;
        break;
      }
      case 0x7C: {
        localVar.wasmType = WasmType::F64;
        break;
      }
      default: {
        std::cout << "met unknown local var wasm typeexit.";
        exit(1);
      }
      }
      while (localVarRepeatTimes-- > 0) {
        localVars.emplace_back(localVar);
      }
      index++;
    }
    moduleInfo.functionsLocalVars.emplace_back(std::move(localVars));
    // localvars save end ,start wasm opCode save
    uint32_t opCodeNums = functionBodySize - (index - localVarSizeIndex);
    std::vector<uint8_t> functionInstructions;
    while (opCodeNums-- > 0) {
      functionInstructions.emplace_back(byteStream[index++]);
    }
    moduleInfo.functionsInstructions.emplace_back(std::move(functionInstructions));
  }
}

std::vector<uint8_t> readFileToByteStream(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::binary);

  if (!file) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  file.seekg(0, std::ios::end);
  const std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);

  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    throw std::runtime_error("Failed to read file: " + filePath);
  }

  return buffer;
}

ModuleInfo processWasmFile(const char *filePath) {
  std::vector<uint8_t> byteStream = readFileToByteStream(filePath);

  std::cout << "print bytestream for file: " << filePath << std::endl;
  for (uint8_t const byte : byteStream) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ' ';
  }
  std::cout << std::dec << std::endl; // Reset to decimal output

  if (byteStream.size() < 8) {
    std::cout << "File content read into byte stream:" << std::endl;
    exit(1);
  }

  ModuleInfo moduleInfo;
  size_t byteIndex = 0;
  byteIndex += 8;

  while (byteIndex < byteStream.size()) {
    switch (static_cast<WASMSectionType>(byteStream[byteIndex])) {
    case WASMSectionType::CUSTOM:
    case WASMSectionType::IMPORT:
    case WASMSectionType::TABLE:
    case WASMSectionType::MEMORY:
    case WASMSectionType::GLOBAL:
    case WASMSectionType::START:
    case WASMSectionType::ELEM:
    case WASMSectionType::DATA: {
      byteIndex++;
      uint32_t const sectionSize = readULEB128(byteStream, byteIndex);
      byteIndex += sectionSize; // cut sectionContent
      break;
    }
    case WASMSectionType::EXPORT: {
      byteIndex++;
      parseExportSection(byteStream, byteIndex, moduleInfo);
      break;
    }
    case WASMSectionType::TYPE: {
      byteIndex++;
      parseTypeSection(byteStream, byteIndex, moduleInfo);
      break;
    }
    case WASMSectionType::FUNCTION: {
      byteIndex++;
      parseFunctionSection(byteStream, byteIndex, moduleInfo);
      break;
    }
    case WASMSectionType::CODE: {
      byteIndex++;
      parseCodeSection(byteStream, byteIndex, moduleInfo);
    }
    default:
      break;
    }
  }

  std::cout << "wasm file :" << filePath << " parse end. got ModuleInfo." << std::endl;

  return moduleInfo;
}
// compile opCode

// 解析函数签名,并分配寄存器和内存，保存相关信息到functionInfo
std::vector<ModuleInfo::LocalVar> parseFuncSignature(const std::string &signature, ModuleInfo::FunctionInfo &funcInfo) {
  std::vector<ModuleInfo::LocalVar> funcParms;
  for (int i = 1; i < signature.size(); i++) {
    ModuleInfo::LocalVar funcParm;
    switch (signature[i]) {
    case 'i': {
      funcParm.wasmType = WasmType::I32;
      funcParm.reg = static_cast<TReg>(funcInfo.numLocalsInGPR++);
      funcInfo.numParams++;
      funcInfo.numLocals++;
      break;
    }
    case 'I': {
      funcParm.wasmType = WasmType::I64;
      funcParm.reg = static_cast<TReg>(funcInfo.numLocalsInGPR++);
      funcInfo.numParams++;
      funcInfo.numLocals++;
      break;
    }
    case 'f': {
      funcParm.wasmType = WasmType::F32;
      funcParm.reg = static_cast<TReg>(funcInfo.numLocalsInFPR++);
      funcInfo.numParams++;
      funcInfo.numLocals++;
      break;
    }
    case 'F': {
      funcParm.wasmType = WasmType::F64;
      funcParm.reg = static_cast<TReg>(funcInfo.numLocalsInFPR++);
      funcInfo.numParams++;
      funcInfo.numLocals++;
      break;
    }
    case ')': {
      funcParm.wasmType = WasmType::INVALID;
      i += 2; // break loop as only one return type (to do)
      break;
    }
    default: {
      break;
    }
    };
    if (funcParm.wasmType == WasmType::INVALID) {
      break;
    }
    funcParms.emplace_back(funcParm);
  }
  return funcParms;
}

// parse that already has wasmType (not function parms)
void parseFuncLocalVars(std::vector<ModuleInfo::LocalVar> &funcLocalVars, ModuleInfo::FunctionInfo &funcInfo) {
  for (auto &localVar : funcLocalVars) {
    switch (localVar.wasmType) {
    case WasmType::I32: {
      localVar.reg = static_cast<TReg>(funcInfo.numLocalsInGPR++);
      funcInfo.numLocals++;
      break;
    }
    case WasmType::I64: {
      localVar.reg = static_cast<TReg>(funcInfo.numLocalsInGPR++);
      funcInfo.numLocals++;
      break;
    }
    case WasmType::F32: {
      localVar.reg = static_cast<TReg>(funcInfo.numLocalsInFPR++);
      funcInfo.numLocals++;
      break;
    }
    case WasmType::F64: {
      localVar.reg = static_cast<TReg>(funcInfo.numLocalsInFPR++);
      funcInfo.numLocals++;
      break;
    }
    default: {
      std::cout << "error: unknown local var wasm type" << std::endl;
      exit(1);
      break;
    }
    };
  }
}

std::vector<uint8_t> parseOpCode(const std::vector<uint8_t> &functionInstructionsCode, size_t index, const size_t funcIndex, ModuleInfo &moduleInfo) {
  Stack stack;
  AArch64_Assembler assembler(moduleInfo);

  // to do init all local variables
  size_t const everInitlocalVariableIndex = moduleInfo.functionInfos[funcIndex].numParams;
  for (size_t j = everInitlocalVariableIndex; j < moduleInfo.functionsLocalVars[funcIndex].size(); ++j) {
    auto reg = moduleInfo.functionsLocalVars[funcIndex][j].reg;
    switch (moduleInfo.functionsLocalVars[funcIndex][j].wasmType) {
    case WasmType::I32: {
      assembler.MOVimm(false, reg, 0);
      break;
    }
    case WasmType::I64: {
      assembler.MOVimm(true, reg, 0);
      break;
    }
    default: {
      throw std::runtime_error("Unsupport wasm type currently.");
    }
    }
  }

  for (size_t i = index; i < functionInstructionsCode.size();) {
    switch (static_cast<OPCode>(functionInstructionsCode[i])) {
    case OPCode::I32_CONST: {
      i++;
      uint32_t i32ConstValue = readULEB128(functionInstructionsCode, i);
      StackElement stackElement;
      stackElement.type = StackType::CONSTANT_I32;
      stackElement.data.constUnion.u32 = i32ConstValue;
      stack.push(stackElement);
      break;
    }
    case OPCode::I64_CONST: {
      i++;
      uint64_t i64ConstValue = readULEB128(functionInstructionsCode, i);
      StackElement stackElement;
      stackElement.type = StackType::CONSTANT_I64;
      stackElement.data.constUnion.u64 = i64ConstValue;
      stack.push(stackElement);
      break;
    }
    case OPCode::LOCAL_GET: {
      i++;
      uint32_t localIndex = readULEB128(functionInstructionsCode, i);
      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = localIndex;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][localIndex].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::LOCAL_SET: { // pop stack and set value
      i++;
      uint32_t localIndex = readULEB128(functionInstructionsCode, i);
      if (stack.empty()) {
        std::cout << "error: stack is empty, parse LOCAL_SET error" << std::endl;
        exit(1);
      }
      const StackElement &stackElement = stack.top();
      switch (static_cast<uint32_t>(stackElement.type)) {
      case StackType::CONSTANT_I32: {
        auto const constValue = stackElement.data.constUnion.u32;
        assembler.MOVimm(false, moduleInfo.functionsLocalVars[funcIndex][localIndex].reg, constValue);
        break;
      }
      case StackType::CONSTANT_I64: {
        auto const constValue = stackElement.data.constUnion.u64;
        assembler.MOVimm(true, moduleInfo.functionsLocalVars[funcIndex][localIndex].reg, constValue);
        break;
      }
      case StackType::LOCAL: {
        // to do if local var in stack.
        bool is64 = moduleInfo.functionsLocalVars[funcIndex][localIndex].wasmType == WasmType::I64;
        assembler.MOVRegister(is64, moduleInfo.functionsLocalVars[funcIndex][localIndex].reg,
                              moduleInfo.functionsLocalVars[funcIndex][stackElement.variableData.location.localIdx].reg);
        break;
      }
      default: {
        throw std::runtime_error("Error: unknown op code");
      }
      }
      stack.pop();
      break;
    }
    case OPCode::LOCAL_TEE: {
      i++;
      uint32_t localIndex = readULEB128(functionInstructionsCode, i);
      if (stack.empty()) {
        std::cout << "error: stack is empty, parse LOCAL_SET error" << std::endl;
        exit(1);
      }
      const StackElement &stackElement = stack.top();
      switch (static_cast<uint32_t>(stackElement.type)) {
      case StackType::CONSTANT_I32: {
        auto const constValue = stackElement.data.constUnion.u32;
        assembler.MOVimm(false, moduleInfo.functionsLocalVars[funcIndex][localIndex].reg, constValue);
        break;
      }
      case StackType::CONSTANT_I64: {
        auto const constValue = stackElement.data.constUnion.u64;
        assembler.MOVimm(true, moduleInfo.functionsLocalVars[funcIndex][localIndex].reg, constValue);
        break;
      }
      case StackType::LOCAL: {
        // to do if local var in stack.
        bool is64 = moduleInfo.functionsLocalVars[funcIndex][localIndex].wasmType == WasmType::I64;
        assembler.MOVRegister(is64, moduleInfo.functionsLocalVars[funcIndex][localIndex].reg,
                              moduleInfo.functionsLocalVars[funcIndex][stackElement.variableData.location.localIdx].reg);
        break;
      }
      default: {
        throw std::runtime_error("Error: unknown op code");
      }
      }
      break;
    }
    case OPCode::END: {
      i++;
      if (!stack.empty()) {
        const StackElement &stackElement = stack.top();
        switch (static_cast<uint32_t>(stackElement.type)) {
        case StackType::CONSTANT_I32: {
          auto const constValue = stackElement.data.constUnion.u32;
          assembler.MOVimm(false, TReg::R0, constValue);
          break;
        }
        case StackType::CONSTANT_I64: {
          auto const constValue = stackElement.data.constUnion.u64;
          assembler.MOVimm(true, TReg::R0, constValue);
          break;
        }
        case StackType::LOCAL: {
          // to do if local var in stack.
          bool is64 = moduleInfo.functionsLocalVars[funcIndex][stackElement.variableData.location.localIdx].wasmType == WasmType::I64;
          assembler.MOVRegister(is64, TReg::R0, moduleInfo.functionsLocalVars[funcIndex][stackElement.variableData.location.localIdx].reg);
          auto vec = assembler.getInstructions();
          break;
        }
        default: {
          std::cout << "error: stackElement type" << std::endl;
          exit(1);
          break;
        }
        }
        stack.pop();
      }
      assembler.Ret();
      break;
    }
    case OPCode::I32_ADD: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I32_ADD wasm opCode error.");
      }
      std::cout << "GKB ADDED" << std::endl;
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I32_ADD wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      if (returnType == WasmType::I32) {
        assembler.AddShiftedRegister(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      } else {
        auto leftLocalVarType = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].wasmType;
        auto rightLocalVarType = moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].wasmType;
        if (rightLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
        } else {
          throw std::runtime_error("error: I32_ADD right type is not I32 or I64, parse I32_ADD wasm opCode error.");
        }
        if (leftLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg);
        } else {
          throw std::runtime_error("error: I32_ADD left type is not I32 or I64, parse I32_ADD wasm opCode error.");
        }
        assembler.AddShiftedRegister(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      }

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I32_SUB: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I32_SUB wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I32_SUB wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      if (returnType == WasmType::I32) {
        assembler.SubShiftedRegister(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      } else {
        auto leftLocalVarType = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].wasmType;
        auto rightLocalVarType = moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].wasmType;
        if (rightLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
        } else {
          throw std::runtime_error("error: I32_SUB right type is not I32 or I64, parse I32_SUB wasm opCode error.");
        }
        if (leftLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg);
        } else {
          throw std::runtime_error("error: I32_SUB left type is not I32 or I64, parse I32_SUB wasm opCode error.");
        }
        assembler.SubShiftedRegister(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      }

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I32_MUL: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I32_MUL wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I32_MUL wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      if (returnType == WasmType::I32) {
        assembler.Multiply(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                           moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      } else {
        auto leftLocalVarType = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].wasmType;
        auto rightLocalVarType = moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].wasmType;
        if (rightLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
        } else {
          throw std::runtime_error("error: I32_MUL right type is not I32 or I64, parse I32_MUL wasm opCode error.");
        }
        if (leftLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg);
        } else {
          throw std::runtime_error("error: I32_MUL left type is not I32 or I64, parse I32_MUL wasm opCode error.");
        }
        assembler.Multiply(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                           moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      }

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I64_ADD: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I64_ADD wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I64_ADD wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      if (returnType == WasmType::I32) {
        assembler.AddShiftedRegister(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      } else {
        auto leftLocalVarType = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].wasmType;
        auto rightLocalVarType = moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].wasmType;
        if (rightLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
        } else if (rightLocalVarType != WasmType::I64) {
          throw std::runtime_error("error: I64_ADD right type is not I32 or I64, parse I64_ADD wasm opCode error.");
        }
        if (leftLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg);
        } else if (rightLocalVarType != WasmType::I64) {
          throw std::runtime_error("error: I64_ADD left type is not I32 or I64, parse I64_ADD wasm opCode error.");
        }
        assembler.AddShiftedRegister(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      }

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I64_SUB: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I64_SUB wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I64_SUB wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      if (returnType == WasmType::I32) {
        assembler.SubShiftedRegister(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      } else {
        auto leftLocalVarType = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].wasmType;
        auto rightLocalVarType = moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].wasmType;
        if (rightLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
        } else if (rightLocalVarType != WasmType::I64) {
          throw std::runtime_error("error: I64_SUB right type is not I32 or I64, parse I64_SUB wasm opCode error.");
        }
        if (leftLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg);
        } else if (rightLocalVarType != WasmType::I64) {
          throw std::runtime_error("error: I64_SUB left type is not I32 or I64, parse I64_SUB wasm opCode error.");
        }
        assembler.SubShiftedRegister(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      }

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I64_MUL: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I64_MUL wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I64_MUL wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      if (returnType == WasmType::I32) {
        assembler.Multiply(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                           moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      } else {
        auto leftLocalVarType = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].wasmType;
        auto rightLocalVarType = moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].wasmType;
        if (rightLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
        } else if (rightLocalVarType != WasmType::I64) {
          throw std::runtime_error("error: I64_MUL right type is not I32 or I64, parse I64_MUL wasm opCode error.");
        }
        if (leftLocalVarType == WasmType::I32) {
          assembler.Sxtw(moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                         moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg);
        } else if (rightLocalVarType != WasmType::I64) {
          throw std::runtime_error("error: I64_MUL left type is not I32 or I64, parse I64_MUL wasm opCode error.");
        }
        assembler.Multiply(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                           moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);
      }

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I64_DIV_S: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I64_DIV_S wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I64_DIV_S wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      assembler.SDIV(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I64_DIV_U: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I64_DIV_U wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I64_DIV_U wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      assembler.UDIV(true, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I32_DIV_S: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I32_DIV_S wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I32_DIV_S wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      assembler.SDIV(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    case OPCode::I32_DIV_U: {
      i++;
      if (stack.size() < 2) {
        throw std::runtime_error("error: stack size less than 2, parse I32_DIV_U wasm opCode error.");
      }
      StackElement right = stack.top();
      stack.pop();
      StackElement left = stack.top();
      stack.pop();
      if (static_cast<uint32_t>(left.type) != StackType::LOCAL || static_cast<uint32_t>(right.type) != StackType::LOCAL) {
        throw std::runtime_error("error: stack element type is not LOCAL, parse I32_DIV_U wasm opCode error.");
      }
      auto returnType = moduleInfo.getReturnTypeForSignature(moduleInfo.functionInfos[funcIndex].typeIndex);

      assembler.UDIV(false, moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg,
                     moduleInfo.functionsLocalVars[funcIndex][right.variableData.location.localIdx].reg);

      StackElement stackElement;
      stackElement.type = StackType::LOCAL;

      StackElement::VariableData data;
      stackElement.variableData.location.localIdx = left.variableData.location.localIdx;
      stackElement.variableData.location.reg = moduleInfo.functionsLocalVars[funcIndex][left.variableData.location.localIdx].reg;
      stack.push(stackElement);
      break;
    }
    default: {
      std::stringstream ss;
      ss << "error: unknown op code is " << static_cast<uint32_t>(functionInstructionsCode[i]);
      std::string errorMessage = ss.str();
      throw std::runtime_error(errorMessage);
      break;
    }
    }
  }
  return assembler.getInstructions();
}

void compileOpCode(ModuleInfo &moduleInfo) {
  std::cout << "Start compile wasm module using ModuleInfo." << std::endl;

  for (int i = 0; i < moduleInfo.functionsInstructions.size(); i++) {
    auto &singlefunctionLocalVars = moduleInfo.functionsLocalVars[i];
    std::string &functionSignatureType = moduleInfo.signatureTypes[moduleInfo.functionInfos[i].typeIndex];

    std::vector<ModuleInfo::LocalVar> funcParmLocals = parseFuncSignature(functionSignatureType, moduleInfo.functionInfos[i]);
    parseFuncLocalVars(singlefunctionLocalVars, moduleInfo.functionInfos[i]);
    funcParmLocals.insert(funcParmLocals.end(), moduleInfo.functionsLocalVars[i].begin(), moduleInfo.functionsLocalVars[i].end());
    moduleInfo.functionsLocalVars[i] = std::move(funcParmLocals);

    auto funcMachineCodes = parseOpCode(moduleInfo.functionsInstructions[i], 0, i, moduleInfo);

    if (funcMachineCodes.empty()) {
      std::stringstream ss;
      ss << "Parse wasm func opCode error , got empty arm64 instructions. wasm func index is: " << i;
      throw std::runtime_error(ss.str());
    }
    moduleInfo.machineCodes.emplace_back(funcMachineCodes);
  }
}