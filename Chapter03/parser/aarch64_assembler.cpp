#include <iostream>

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
  // uint32_t instruction = 0;
  // if (!is64) {
  //   instruction = 0x52800000; // MOVZ Wd, #imm16
  // } else {
  //   instruction = 0xD2800000; // MOVZ Xd, #imm16
  // }

  MOVK(is64, reg, static_cast<uint16_t>(imm & 0xFFFFU), 0);          // 第 0-15 位
  MOVK(is64, reg, static_cast<uint16_t>((imm >> 16U) & 0xFFFFU), 1); // 第 16-31 位
  if (is64) {
    MOVK(is64, reg, static_cast<uint16_t>((imm >> 32U) & 0xFFFFU), 2); // 第 32-47 位
    MOVK(is64, reg, static_cast<uint16_t>((imm >> 48U) & 0xFFFFU), 3); // 第 48-63 位
  }

  // instruction |= (imm16 << 5U);
  // instruction |= static_cast<uint8_t>(reg);

  // insertInstructionIntoVector(instruction, this->instructions_);
}

// hw = shift , 0 = 0 , 1 = 16 , 2 = 32 , 3 = 48
void AArch64_Assembler::MOVK(bool const is64, TReg const reg, uint16_t const imm16, uint8_t const hw) {
  uint32_t instruction = 0;
  if (!is64) {
    instruction = 0x72800000U;
  } else {
    instruction = 0xF2800000U;
  }
  instruction |= (static_cast<uint32_t>(hw) << 21U);
  instruction |= (static_cast<uint32_t>(imm16) << 5U);
  instruction |= static_cast<uint8_t>(reg);
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::MOVRegister(bool is64, TReg const dst, TReg const src) {
  uint32_t instruction;
  if (is64) {
    instruction = 0xAA000000U;
  } else {
    instruction = 0x2A000000U;
  }

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
