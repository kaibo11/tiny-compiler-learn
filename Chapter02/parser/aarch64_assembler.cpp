#include <iostream>

#include "StackElement.hpp"
#include "aarch64_assembler.hpp"

AArch64_Assembler::AArch64_Assembler(ModuleInfo &moduleInfo) : moduleInfo_(moduleInfo) {
}
void AArch64_Assembler::MOVimm(bool const is64, TReg const reg, uint64_t const imm) {
  // assert(RegUtil::isGPR(reg) && "Only GPR registers allowed");
  uint32_t instruction = 0;
  if (!is64) {
    instruction = 0x52800000; // MOVZ Wd, #imm16
  } else {
    instruction = 0xD2800000; // MOVZ Xd, #imm16
  }

  uint8_t hw = 0;                              // to do implement
  uint32_t imm16 = static_cast<uint32_t>(imm); // to do implement

  static_cast<void>(hw);

  instruction |= (imm16 << 5U);
  instruction |= static_cast<uint8_t>(reg);
  this->instructions_.emplace_back(instruction);
}

inline void AArch64_Assembler::MOVRegister(TReg const dst, TReg const src) {
  uint32_t instruction = 0x2A000000U;

  uint8_t shift = 0; // to do implement
  static_cast<void>(shift);
}