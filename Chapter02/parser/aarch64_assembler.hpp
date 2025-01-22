#include <cstdint>
#include <sys/mman.h>
#include <vector>

#include "ModuleInfo.hpp"
#include "aarch64_common.hpp"

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
  void MOVRegister(TReg const dst, TReg const src);

  void Ret();

  std::vector<uint8_t> getInstructions() {
    return instructions_;
  }

private:
  void insertInstructionIntoVector(uint32_t instruction, std::vector<uint8_t> &vec) {
    vec.push_back(static_cast<uint8_t>(instruction & 0xFFU));
    vec.push_back(static_cast<uint8_t>((instruction >> 8U) & 0xFFU));
    vec.push_back(static_cast<uint8_t>((instruction >> 16U) & 0xFFU));
    vec.push_back(static_cast<uint8_t>((instruction >> 24U) & 0xFFU));
  }
  std::vector<uint8_t> instructions_;
  ModuleInfo &moduleInfo_;
};