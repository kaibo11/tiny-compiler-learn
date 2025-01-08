#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>

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
  static_cast<void>(moduleInfo);
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

ModuleInfo processWasmFile(char *filePath) {
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
    case WASMSectionType::EXPORT:
    case WASMSectionType::START:
    case WASMSectionType::ELEM:
    case WASMSectionType::DATA: {
      byteIndex++;
      uint32_t const sectionSize = readULEB128(byteStream, byteIndex);
      byteIndex += sectionSize; // cut sectionContent
      break;
    }
    case WASMSectionType::TYPE: {
      byteIndex++;
      parseTypeSection(byteStream, byteIndex, moduleInfo);
      exit(1);
      break;
    }
    case WASMSectionType::FUNCTION: {
      byteIndex++;
      uint32_t const sectionSize = readULEB128(byteStream, byteIndex);
      byteIndex += sectionSize; // cut sectionContent
      break;
    }
    case WASMSectionType::CODE: {
      byteIndex++;
      uint32_t const sectionSize = readULEB128(byteStream, byteIndex);
      static_cast<void>(sectionSize);
      uint32_t const functionSize = readULEB128(byteStream, byteIndex);
      static_cast<void>(functionSize); // cut func size , assume only one function
      uint32_t const functionCodeSize = readULEB128(byteStream, byteIndex);
      uint32_t const localVarIndex = byteIndex;
      uint32_t const localVarSize = readULEB128(byteStream, byteIndex); // assume it should 0
      static_cast<void>(localVarSize);
      uint32_t const opCodeIndex = byteIndex;
      auto opCodeSize = localVarIndex + functionCodeSize - opCodeIndex;
      byteIndex += opCodeSize;
    }
    default:
      break;
    }
  }

  std::cout << "wasm file :" << filePath << " parse end." << std::endl;

  return moduleInfo;
}