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
#include "aarch64_common.hpp"
#include "parser.hpp"

class AArch64_Assembler {
public:
  explicit AArch64_Assembler(ModuleInfo &moduleInfo);
  void MOVimm(bool const is64, TReg const reg, uint64_t const imm);

  inline void MOVimm32(TReg const reg, uint32_t const imm) {
    MOVimm(false, reg, static_cast<uint64_t>(imm));
  }

  inline void MOVimm64(TReg const reg, uint64_t const imm) {
    MOVimm(true, reg, imm);
  }

  // only support mov register to register
  inline void MOVRegister(TReg const dst, TReg const src);

private:
  std::vector<uint32_t> instructions_;
  ModuleInfo &moduleInfo_;
};