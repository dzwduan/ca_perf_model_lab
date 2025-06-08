// Copyright 2024 Blaise Tine
//
// Licensed under the Apache License;
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <assert.h>
#include <util.h>
#include "FU.h"
#include "core.h"

using namespace tinyrv;

static uint32_t execute_alu_op(const Instr &instr, uint32_t rs1_data, uint32_t rs2_data) {
  auto exe_flags  = instr.getExeFlags();
  auto alu_op     = instr.getAluOp();

  uint32_t alu_s1 = exe_flags.alu_s1_PC ? instr.getPC() : (exe_flags.alu_s1_rs1 ? instr.getRs1() :  rs1_data);
  uint32_t alu_s2 = exe_flags.alu_s2_imm ? instr.getImm() : rs2_data;

  if (exe_flags.alu_s1_inv) {
    alu_s1 = ~alu_s1;
  }

  uint32_t rd_data = 0;

  switch (alu_op) {
  case AluOp::NONE:
    break;
  case AluOp::ADD: {
    rd_data = alu_s1 + alu_s2;
    break;
  }
  case AluOp::SUB: {
    rd_data = alu_s1 - alu_s2;
    break;
  }
  case AluOp::AND: {
    rd_data = alu_s1 & alu_s2;
    break;
  }
  case AluOp::OR: {
    rd_data = alu_s1 | alu_s2;
    break;
  }
  case AluOp::XOR: {
    rd_data = alu_s1 ^ alu_s2;
    break;
  }
  case AluOp::SLL: {
    rd_data = alu_s1 << alu_s2;
    break;
  }
  case AluOp::SRL: {
    rd_data = alu_s1 >> alu_s2;
    break;
  }
  case AluOp::SRA: {
    rd_data = (int32_t)alu_s1 >> alu_s2;
    break;
  }
  case AluOp::LTI: {
    rd_data = (int32_t)alu_s1 < (int32_t)alu_s2;
    break;
  }
  case AluOp::LTU: {
    rd_data = alu_s1 < alu_s2;
    break;
  }
  default:
    std::abort();
  }

  return rd_data;
}

///////////////////////////////////////////////////////////////////////////////

static uint32_t execute_br_op(BrOp br_op, uint32_t rs1_data, uint32_t rs2_data) {
  bool br_taken = false;

  switch (br_op) {
  case BrOp::NONE:
    break;
  case BrOp::JAL:
  case BrOp::JALR: {
    br_taken = true;
    break;
  }
  case BrOp::BEQ: {
    br_taken = (rs1_data == rs2_data);
    break;
  }
  case BrOp::BNE: {
    br_taken = (rs1_data != rs2_data);
    break;
  }
  case BrOp::BLT: {
    br_taken = ((int32_t)rs1_data < (int32_t)rs2_data);
    break;
  }
  case BrOp::BGE: {
    br_taken = ((int32_t)rs1_data >= (int32_t)rs2_data);
    break;
  }
  case BrOp::BLTU: {
    br_taken = (rs1_data < rs2_data);
    break;
  }
  case BrOp::BGEU: {
    br_taken = (rs1_data >= rs2_data);
    break;
  }
  default:
    std::abort();
  }

  return br_taken;
}

///////////////////////////////////////////////////////////////////////////////

void ALU::do_execute() {
  result_ = execute_alu_op(*instr_, rs1_value_, rs2_value_);
}

void BRU::do_execute() {
  auto br_op = instr_->getBrOp();
  auto br_taken = execute_br_op(br_op, rs1_value_, rs2_value_);
  if (br_taken) {
    auto br_target = execute_alu_op(*instr_, rs1_value_, rs2_value_);
    core_->PC_ = br_target;
    if (br_op == BrOp::JAL || br_op == BrOp::JALR) {
      result_ = instr_->getPC() + 4; // return PC + 4
    }
  }
  DT(2, "Branch: " << (br_taken ? "taken" : "not-taken") << ", target=0x" << std::hex << core_->PC_ << std::dec << " (#" << instr_->getId() << ")");
  core_->fetch_stalled_->write(false); // release fetch stage
}

void LSU::do_execute() {
  auto exe_flags = instr_->getExeFlags();
  auto func3 = instr_->getFunc3();

  if (exe_flags.is_load) {
    uint64_t mem_addr = execute_alu_op(*instr_, rs1_value_, rs2_value_);
    uint32_t data_bytes = 1 << (func3 & 0x3);
    uint32_t data_width = 8 * data_bytes;
    uint32_t read_data = 0;
    core_->dmem_read(&read_data, mem_addr, data_bytes);
    switch (func3) {
    case 0: // RV32I: LB
    case 1: // RV32I: LH
      result_ = sext(read_data, data_width);
      break;
    case 2: // RV32I: LW
      result_ = sext(read_data, data_width);
      break;
    case 4: // RV32I: LBU
    case 5: // RV32I: LHU
      result_ = read_data;
      break;
    default:
      std::abort();
    }
  } else if (exe_flags.is_store) {
    uint64_t mem_addr = execute_alu_op(*instr_, rs1_value_, rs2_value_);
    uint32_t data_bytes = 1 << (func3 & 0x3);
    switch (func3) {
    case 0:
    case 1:
    case 2:
      core_->dmem_write(&rs2_value_, mem_addr, data_bytes);
      break;
    default:
      std::abort();
    }
  }
}

void SFU::do_execute() {
  auto csr_data = core_->get_csr(instr_->getImm());
  auto rd_data = execute_alu_op(*instr_, rs1_value_, csr_data);
  if (rd_data != csr_data) {
    core_->set_csr(instr_->getImm(), rd_data);
  }
  result_ = csr_data;
}
