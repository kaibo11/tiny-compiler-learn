#include "aarch64_assembler.hpp"

AArch64_Assembler::AArch64_Assembler(ModuleInfo &moduleInfo) : moduleInfo_(moduleInfo) {
}

void AArch64_Assembler::insertInstructionIntoVector(uint32_t instruction, std::vector<uint8_t> &vec) {
  vec.push_back(static_cast<uint8_t>(instruction & 0xFFU));
  vec.push_back(static_cast<uint8_t>((instruction >> 8U) & 0xFFU));
  vec.push_back(static_cast<uint8_t>((instruction >> 16U) & 0xFFU));
  vec.push_back(static_cast<uint8_t>((instruction >> 24U) & 0xFFU));
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

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::MOVRegister(TReg const dst, TReg const src) {
  uint32_t instruction = 0x2A000000U;

  uint8_t shift = 0; // to do implement
  static_cast<void>(shift);
  auto Rm = src;
  auto Rd = dst;
  uint8_t Rn = 0x1F; // 11111
  uint8_t imm6 = 0;

  instruction |= static_cast<uint8_t>(Rd);
  instruction |= static_cast<uint16_t>(Rn << 5U);
  instruction |= (static_cast<uint32_t>(Rm) << 16U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::AddShiftedRegister(bool is64, TReg const first, TReg const second) {
  uint32_t instruction;
  if (is64) {
    instruction = 0x8B000000U;
  } else {
    instruction = 0x0B000000U;
  }

  uint8_t shift = 0; // to do implement
  static_cast<void>(shift);
  auto Rn = first;
  auto Rm = second;
  auto Rd = first;

  instruction |= static_cast<uint8_t>(Rd);
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(Rm) << 16U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::SubShiftedRegister(bool is64, TReg const first, TReg const second) {
  uint32_t instruction;
  if (is64) {
    instruction = 0xCB000000U;
  } else {
    instruction = 0x4B000000U;
  }

  uint8_t shift = 0; // to do implement
  static_cast<void>(shift);
  auto Rn = first;
  auto Rm = second;
  auto Rd = first;

  instruction |= static_cast<uint8_t>(Rd);
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(Rm) << 16U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::Sxtw(TReg const dst, TReg const src) {
  uint32_t instruction = 0x93407C00U;

  uint8_t shift = 0; // to do implement
  static_cast<void>(shift);
  auto Rn = src;
  auto Rd = dst;

  instruction |= static_cast<uint8_t>(Rd);
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::Multiply(bool is64, TReg const first, TReg const second) {
  uint32_t instruction = 0x9B007C00U;
  if (is64) {
    instruction = 0x9B007C00U;
  } else {
    instruction = 0x1B007C00U;
  }

  uint8_t shift = 0; // to do implement
  static_cast<void>(shift);
  auto Rn = first;
  auto Rm = second;
  auto Rd = first;

  instruction |= static_cast<uint8_t>(Rd);
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(Rm) << 16U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::stpSpecial1() {
  uint32_t instruction = 0xA9BF7BFD;
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::moveSpecial1() {
  uint32_t instruction = 0x910003FD;
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::ldpSpecial1() {
  uint32_t instruction = 0xa8c17bfd;
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::blSpecial1() {
  uint32_t instruction = 0x94000003;
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::Ret() {
  uint32_t instruction = 0xD65F03C0; // RET
  insertInstructionIntoVector(instruction, this->instructions_);
}
