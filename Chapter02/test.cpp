#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "parser/aarch64_assembler.hpp"
#include "parser/aarch64_common.hpp"
#include "parser/parser.hpp"

using json = nlohmann::json;

TEST(JsonTest, ParseJson) {
  std::ifstream ifs("../local.json");
  if (!ifs.is_open()) {
    std::cerr << "Could not open the file!" << std::endl;
    return;
  }

  json j;
  ifs >> j;

  std::cout << "Source filename: " << j["source_filename"].get<std::string>() << std::endl;

  const auto &commands = j["commands"];
  std::string wasmFilePathPrefix = "../";

  ModuleInfo moduleInfo;

  for (const auto &command : commands) {
    std::cout << "Command type: " << command["type"].get<std::string>() << ", line: " << command["line"].get<int>() << std::endl;

    // wasm filename
    if (command.contains("filename")) {
      auto commandFilename = command["filename"].get<std::string>();
      std::cout << "Filename: " << command["filename"].get<std::string>() << std::endl;
      std::string wasmFilePath = wasmFilePathPrefix + commandFilename;
      moduleInfo = processWasmFile(wasmFilePath.data());
      compileOpCode(moduleInfo);
      continue;
    }
    AArch64_Assembler assembler(moduleInfo);
    std::string funcName;
    if (command.contains("action")) {
      std::cout << "Action type: " << command["action"]["type"].get<std::string>() << std::endl;
      std::cout << "Action field: " << command["action"]["field"].get<std::string>() << std::endl;
      funcName = command["action"]["field"].get<std::string>();

      assembler.stpSpecial1();
      assembler.moveSpecial1();

      if (command["action"].contains("args")) {
        const auto &args = command["action"]["args"];
        TReg reg = TReg::R0;

        for (const auto &arg : args) {
          assembler.MOVimm(false, reg, std::stoull(arg["value"].get<std::string>()));
          reg = static_cast<TReg>(static_cast<uint8_t>(reg) + 1);
          std::cout << "Arg type: " << arg["type"].get<std::string>() << ", value: " << arg["value"].get<std::string>() << std::endl;
        }
      }

      assembler.blSpecial1();
    }

    assembler.ldpSpecial1();
    assembler.Ret();

    std::vector<uint8_t> testInstr = assembler.getInstructions();
    auto needTestedFuncIndex = moduleInfo.functionsNameIndex.find(funcName);

    if (needTestedFuncIndex == moduleInfo.functionsNameIndex.end()) {
      std::cout << "func name:" << funcName << " is not found in moduleInfo" << std::endl;
      exit(1);
    }

    auto testFuncInstr = moduleInfo.machineCodes[needTestedFuncIndex->second];
    testInstr.insert(testInstr.end(), testFuncInstr.begin(), testFuncInstr.end());
    std::cout << "test instr is :[";
    for (size_t i = 0; i < testInstr.size(); ++i) {
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(testInstr[i]);
      if (i < testInstr.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << "]" << std::endl;
    if (command.contains("expected")) {
      uint32_t (*func)() = nullptr;
      const auto &expected = command["expected"];
      for (const auto &exp : expected) {
        std::cout << "Expected type: " << exp["type"].get<std::string>() << ", value: " << exp["value"].get<std::string>() << std::endl;
        func =
            reinterpret_cast<uint32_t (*)()>(mmap(nullptr, testInstr.size(), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        memcpy(reinterpret_cast<void *>(func), testInstr.data(), testInstr.size());
        __builtin___clear_cache(reinterpret_cast<char *>(func), reinterpret_cast<char *>(func) + testInstr.size());
        uint32_t result = func();
        ASSERT_EQ(result, std::stoull(exp["value"].get<std::string>()));

        munmap(reinterpret_cast<void *>(func), testInstr.size());
      }
    } else {
      void (*func)() = nullptr;
      func = reinterpret_cast<void (*)()>(mmap(nullptr, testInstr.size(), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
      memcpy(reinterpret_cast<void *>(func), testInstr.data(), testInstr.size());
      __builtin___clear_cache(reinterpret_cast<char *>(func), reinterpret_cast<char *>(func) + testInstr.size());
      func();

      munmap(reinterpret_cast<void *>(func), testInstr.size());
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}