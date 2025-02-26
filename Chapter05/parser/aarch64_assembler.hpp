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

  void MOVK(bool const is64, TReg const reg, uint16_t const imm16, uint8_t const hw);

  inline void MOVimm64(bool const is64, TReg const reg, uint64_t const imm) {
    MOVimm(true, reg, imm);
  }

  void AddShiftedRegister(bool is64, TReg const first, TReg const second);

  void SubShiftedRegister(bool is64, TReg const first, TReg const second);

  void Multiply(bool is64, TReg const first, TReg const second);

  void UDIV(bool is64, TReg const first, TReg const second);

  void SDIV(bool is64, TReg const first, TReg const second);

  void CMP(bool is64, TReg const first, uint16_t imm12);
  // cmp shifted register
  void CMP(bool is64, TReg const first, TReg const second);

  void CMN(bool is64, TReg const first, uint16_t imm12);

  void Bcon(uint8_t const cond, uint32_t offset);

  void BR(TReg const reg);

  void Sxtw(TReg const dst, TReg const src);

  // stp  x29, x30, [sp, -16]!
  void stpSpecial1();

  // mov  x29, sp
  void moveSpecial1();

  // ldp  x29, x30, [sp], 16
  void ldpSpecial1();

  // bl imm28 = 3
  void blSpecial1();

  // only support mov register to register
  void MOVRegister(bool is64, TReg const dst, TReg const src);

  void Ret();

  void B(uint32_t imm26);

  void notifyIfBlock() {
    ifBlockState = 1;
  }

  void notifyElseBlock() {
    ifBlockState = 2;
  }

  void notifyIfEnd() {
    ifBlockState = 0;
  }

  std::vector<uint8_t> getInstructions() {
    return instructions_;
  }

  void notifyIfBlockEnd() {
    instructions_.insert(instructions_.end(), ifBlockInstructions_.begin(), ifBlockInstructions_.end());
    if (elseBlockInstructions_.size() > 0) {
      B(elseBlockInstructions_.size() / 4 + 1); // jump else block
    }
    instructions_.insert(instructions_.end(), elseBlockInstructions_.begin(), elseBlockInstructions_.end());
    ifBlockState = 0;
  }

  // private:
  void insertInstructionIntoVector(uint32_t instruction, std::vector<uint8_t> &vec);
  std::vector<uint8_t> instructions_;
  ModuleInfo &moduleInfo_;

  std::vector<uint8_t> ifBlockInstructions_;
  std::vector<uint8_t> elseBlockInstructions_;

  // 1 = if block, 2 = else block
  uint8_t ifBlockState = 0;
};