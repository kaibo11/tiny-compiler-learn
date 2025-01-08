#ifndef WASM_PARSER_HPP
#define WASM_PARSER_HPP

#include <cstdint>
#include <vector>

#include "ModuleInfo.hpp"

uint32_t readULEB128(const std::vector<uint8_t> &data, size_t &index);

enum class WASMSectionType : uint8_t {
  CUSTOM = 0,
  TYPE = 1,
  IMPORT = 2,
  FUNCTION = 3,
  TABLE = 4,
  MEMORY = 5,
  GLOBAL = 6,
  EXPORT = 7,
  START = 8,
  ELEM = 9,
  CODE = 10,
  DATA = 11
};

void parseTypeSection(const std::vector<uint8_t> &byteStream, size_t &index, ModuleInfo &moduleInfo);

std::vector<uint8_t> readFileToByteStream(const std::string &filePath);

ModuleInfo processWasmFile(char *filePath);

#endif // WASM_PARSER_HPP