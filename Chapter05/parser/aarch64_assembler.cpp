#include <iostream>

#include "aarch64_assembler.hpp"
#include "aarch64_common.hpp"

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

void AArch64_Assembler::BR(TReg const reg) {
  uint32_t instruction = 0xD61F0000U;
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(reg) << 5U);
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::UDIV(bool is64, TReg const first, TReg const second) {
  CMP(is64, second, 0);
  Bcon(1, is64 ? 6 : 4); // 和0 不相等就跳过下一条指令, 也就是跳到trap地址
  MOVimm(is64, TReg::R0, 1);
  BR(TReg::R28); // trap address
  uint32_t instruction;
  if (is64) {
    instruction = 0x9AC00800U;
  } else {
    instruction = 0x1AC00800U;
  }

  auto Rn = first;
  auto Rm = second;
  auto Rd = first;
  instruction |= static_cast<uint8_t>(Rd);
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(Rm) << 16U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::CMP(bool is64, TReg const first, uint16_t imm12) {
  uint32_t instruction;
  if (is64) {
    instruction = 0xF100001FU;
  } else {
    instruction = 0x7100001FU;
  }

  // to do use sh ,current use default 0

  auto Rn = first;
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(imm12) << 10U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::CMP(bool is64, TReg const first, TReg const second) {
  uint32_t instruction;
  if (is64) {
    instruction = 0xEB00001FU;
  } else {
    instruction = 0x6B00001FU;
  }

  uint8_t shift = 0; // to do implement
  uint8_t imm6 = 0;  // to do implement
  static_cast<void>(shift);
  static_cast<void>(imm6);

  auto Rn = first;
  auto Rm = second;
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(Rm) << 16U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::CMN(bool is64, TReg const first, uint16_t imm12) {
  uint32_t instruction;
  if (is64) {
    instruction = 0xB100001FU;
  } else {
    instruction = 0x3100001FU;
  }
  auto Rn = first;
  instruction |= static_cast<uint16_t>(static_cast<uint16_t>(Rn) << 5U);
  instruction |= (static_cast<uint32_t>(imm12) << 10U);

  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::Bcon(uint8_t const cond, uint32_t offset) {
  uint32_t instruction = 0x54000000U;
  instruction |= cond;
  instruction |= (offset << 5U);
  insertInstructionIntoVector(instruction, this->instructions_);
}

void AArch64_Assembler::SDIV(bool is64, TReg const first, TReg const second) {
  uint32_t instruction;
  if (is64) {
    instruction = 0x9AC00C00U;
  } else {
    instruction = 0x1AC00C00U;
  }

  CMP(is64, second, 0);
  Bcon(1, is64 ? 6 : 4); // 和0 不相等就跳过下一条指令,继续判断是否是first最大值 或者-1
  MOVimm(is64, TReg::R0, 1);
  BR(TReg::R28); // trigger trap

  MOVimm(is64, TReg::R26, is64 ? 18446744073709551615U : 4294967295U);
  CMP(is64, second, TReg::R26); // 被除数不能是-1
  Bcon(1, is64 ? 12 : 8);       // 和-1不相等就跳过所有去执行除法， 否则继续判断除数
  MOVimm(is64, TReg::R27, is64 ? 0x8000000000000000U : 0x80000000U);
  CMP(is64, first, TReg::R27);
  Bcon(1, is64 ? 6 : 4);
  MOVimm(is64, TReg::R0, 2);
  BR(TReg::R28); // trap address

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

void AArch64_Assembler::processIfBlocks() {
}
