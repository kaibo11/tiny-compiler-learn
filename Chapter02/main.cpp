#include <cstring>
#include <iostream>

#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <wasm_file1> <wasm_file2> ..." << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; ++i) {
    auto a = processWasmFile(argv[i]);
    compileOpCode(a);
  }

  return 0;
}
