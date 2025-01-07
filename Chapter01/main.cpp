#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
#include <vector>

uint32_t readULEB128(const std::vector<uint8_t> &data, size_t &index) {
  uint32_t result = 0;
  uint32_t shift = 0;
  const int maxBytes = 5; // ULEB128 for 32-bit integers should not exceed 5 bytes

  for (int byteCount = 0; byteCount < maxBytes; ++byteCount) {
    if (index >= data.size()) {
      throw std::out_of_range("ULEB128 encoding is incomplete or data is truncated.");
    }

    uint8_t byte = data[index++];
    result |= (byte & 0x7FU) << shift;

    if ((byte & 0x80U) == 0) {
      // If the highest bit is not set, this is the last byte
      return result;
    }

    shift += 7;
  }

  throw std::overflow_error("ULEB128 encoding exceeds the maximum length for 32-bit integers.");
}

enum class WASMSectionType : uint8_t {
  CUSTOM_SECTION = 0,
  TYPE_SECTION = 1,
  IMPORT_SECTION = 2,
  FUNCTION_SECTION = 3,
  TABLE_SECTION = 4,
  MEMORY_SECTION = 5,
  GLOBAL_SECTION = 6,
  EXPORT_SECTION = 7,
  START_SECTION = 8,
  ELEM_SECTION = 9,
  CODE_SECTION = 10,
  DATA_SECTION = 11
};

std::vector<uint8_t> readFileToByteStream(const std::string &filePath) {
  std::ifstream file(filePath, std::ios::binary);

  if (!file) {
    throw std::runtime_error("Failed to open file: " + filePath);
  }

  file.seekg(0, std::ios::end);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> buffer(size);

  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    throw std::runtime_error("Failed to read file: " + filePath);
  }

  return buffer;
}

void execWasmFuncCode(const std::vector<uint8_t> &data, size_t index, size_t codeSize) {
  std::vector<uint8_t> instr;
  while (codeSize > 0) {
    switch (data[index]) {
    case 0x0b: {
      instr.insert(instr.end(), {0xc0, 0x03, 0x5f, 0xd6});
      break;
    }
    case 0x0f: {
      // instr.insert(instr.end(), {0xc0, 0x03, 0x5f, 0xd6});
      break;
    }
    default:
      return;
    }
    index++;
    codeSize--;
  }
  void (*func)() = nullptr;
  func = reinterpret_cast<void (*)()>(mmap(nullptr, instr.size(), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  memcpy(reinterpret_cast<void *>(func), instr.data(), instr.size());
  __builtin___clear_cache(reinterpret_cast<char *>(func), reinterpret_cast<char *>(func) + instr.size());
  func();

  munmap(reinterpret_cast<void *>(func), instr.size());
}

int main() {
  std::string filePath = "add.wasm";

  std::vector<uint8_t> byteStream = readFileToByteStream(filePath);

  for (uint8_t byte : byteStream) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << ' ';
  }
  std::cout << std::dec << std::endl; // Reset to decimal output

  if (byteStream.size() < 8) {
    std::cout << "File content read into byte stream:" << std::endl;
  }

  size_t byteIndex = 0;
  byteIndex += 8;

  while (byteIndex < byteStream.size()) {
    switch (byteStream[byteIndex]) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 11: {
      byteIndex++;
      uint32_t sectionSize = readULEB128(byteStream, byteIndex);
      byteIndex += sectionSize; // cut sectionContent
      break;
    }
    case 10: {
      byteIndex++;
      uint32_t sectionSize = readULEB128(byteStream, byteIndex);
      static_cast<void>(sectionSize);
      uint32_t functionSize = readULEB128(byteStream, byteIndex);
      static_cast<void>(functionSize); // cut func size , assume only one function
      uint32_t functionCodeSize = readULEB128(byteStream, byteIndex);
      uint32_t localVarIndex = byteIndex;
      uint32_t localVarSize = readULEB128(byteStream, byteIndex); // assume it should 0
      static_cast<void>(localVarSize);
      uint32_t opCodeIndex = byteIndex;
      auto opCodeSize = localVarIndex + functionCodeSize - opCodeIndex;
      execWasmFuncCode(byteStream, opCodeIndex, opCodeSize);
      byteIndex += opCodeSize;
    }
    default:
      break;
    }
  }

  std::cout << "wasm parse end" << std::endl;

  return 0;
}