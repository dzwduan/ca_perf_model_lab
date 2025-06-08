// Copyright 2025 Blaise Tine
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
#include <iomanip>
#include <string.h>
#include <assert.h>
#include <util.h>
#include "types.h"
#include "core.h"
#include "debug.h"
#include "processor_impl.h"

using namespace tinyrv;

Core::Core(const SimContext& ctx, uint32_t core_id, ProcessorImpl* processor)
    : SimObject(ctx, "core")
    , core_id_(core_id)
    , processor_(processor)
    , reg_file_(NUM_REGS)
    , decode_queue_(FiFoReg<id_data_t>::Create("idq"))
    , issue_queue_(FiFoReg<is_data_t>::Create("isq"))
    , fetch_stalled_(ValReg<bool>::Create("fetch_stalled", false))
    , ROB_(/*TODO: use size info from config.h*/)
    , RAT_(/*TODO: use size info from config.h*/)
    , RS_(/*TODO: use size info from config.h*/)
    , RST_(/*TODO: use size info from config.h*/)
    , FUs_(/*TODO: use size info from config.h*/)
{
  // create functional units
  FUs_.at((int)FUType::ALU) = std::make_shared<ALU>(this);
  FUs_.at((int)FUType::LSU) = std::make_shared<LSU>(this);
  FUs_.at((int)FUType::BRU) = std::make_shared<BRU>(this);
  FUs_.at((int)FUType::SFU) = std::make_shared<SFU>(this);

  // initialize register file at x0
  reg_file_.at(0) = 0;

  this->reset();
}

Core::~Core() {}

void Core::reset() {
  decode_queue_->reset();
  issue_queue_->reset();

  PC_ = STARTUP_ADDR;

  uuid_ctr_ = 0;

  fetched_instrs_ = 0;
  perf_stats_ = PerfStats();

  fetch_stalled_->reset();
  exited_ = false;
}

void Core::tick() {
  this->commit();
  this->writeback();
  this->execute();
  this->issue();
  this->decode();
  this->fetch();

  ++perf_stats_.cycles;
  DPN(2, std::flush);
}

void Core::fetch() {
  if (fetch_stalled_->read() || decode_queue_->full())
    return;

  // allocate a new uuid
  uint32_t uuid = uuid_ctr_++;

  // fetch next instruction from memory at PC address
  uint32_t instr_code = 0;
  mmu_.read(&instr_code, PC_, sizeof(uint32_t), 0);

  DT(2, "Fetch: instr=0x" << instr_code << ", PC=0x" << std::hex << PC_ << std::dec << " (#" << uuid << ")");

  // move instruction data to next stage
  decode_queue_->push({instr_code, PC_, uuid});

  // advance program counter
  PC_ += 4;

  ++fetched_instrs_;

  // This pipeline has no support for branch prediction,
  // we should all the fetch stage until decode
  fetch_stalled_->write(true);
}

void Core::decode() {
  if (decode_queue_->empty() || issue_queue_->full())
    return;

  auto& id_data = decode_queue_->data();

  // instruction decode
  auto instr = this->decode(id_data.instr_code, id_data.PC, id_data.uuid);

  DT(2, "Decode: " << *instr);

  // release fetch stage if not a branch
  // keep fetch stage locked if exiting program
  if (instr->getBrOp() == BrOp::NONE
   && !instr->getExeFlags().is_exit) {
    fetch_stalled_->write(false); // unlock fetch stage
  }

  // move instruction data to next stage
  issue_queue_->push({instr});
  decode_queue_->pop();
}

void Core::dmem_read(void *data, uint64_t addr, uint32_t size) {
  auto type = get_addr_type(addr);
  __unused (type);
  mmu_.read(data, addr, size, 0);
  DT(2, "Mem Read: addr=0x" << std::hex << addr << ", data=0x" << ByteStream(data, size) << " (size=" << size << ", type=" << type << ")");
}

void Core::dmem_write(const void* data, uint64_t addr, uint32_t size) {
  auto type = get_addr_type(addr);
  __unused (type);
  if (addr >= uint64_t(IO_COUT_ADDR)
   && addr < (uint64_t(IO_COUT_ADDR) + IO_COUT_SIZE)) {
     this->writeToStdOut(data);
  } else {
    mmu_.write(data, addr, size, 0);
  }
  DT(2, "Mem Write: addr=0x" << std::hex << addr << ", data=0x" << ByteStream(data, size) << " (size=" << size << ", type=" << type << ")");
}

uint32_t Core::get_csr(uint32_t addr) {
  // stall-independent mcycle workaround for software timing consistency
  uint64_t ideal_mcycles = (perf_stats_.instrs-1) + 5;
  switch (addr) {
  case VX_CSR_MHARTID:
  case VX_CSR_SATP:
  case VX_CSR_PMPCFG0:
  case VX_CSR_PMPADDR0:
  case VX_CSR_MSTATUS:
  case VX_CSR_MISA:
  case VX_CSR_MEDELEG:
  case VX_CSR_MIDELEG:
  case VX_CSR_MIE:
  case VX_CSR_MTVEC:
  case VX_CSR_MEPC:
  case VX_CSR_MNSTATUS:
    return 0;
  case VX_CSR_MCYCLE: // NumCycles
    return ideal_mcycles & 0xffffffff;
  case VX_CSR_MCYCLE_H: // NumCycles
    return (uint32_t)(ideal_mcycles >> 32);
  case VX_CSR_MINSTRET: // NumInsts
    return perf_stats_.instrs & 0xffffffff;
  case VX_CSR_MINSTRET_H: // NumInsts
    return (uint32_t)(perf_stats_.instrs >> 32);
  default:
    std::cout << std::hex << "Error: invalid CSR read addr=0x" << addr << std::endl;
    std::abort();
    return 0;
  }
}

void Core::set_csr(uint32_t addr, uint32_t value) {
  switch (addr) {
  case VX_CSR_SATP:
  case VX_CSR_MSTATUS:
  case VX_CSR_MEDELEG:
  case VX_CSR_MIDELEG:
  case VX_CSR_MIE:
  case VX_CSR_MTVEC:
  case VX_CSR_MEPC:
  case VX_CSR_PMPCFG0:
  case VX_CSR_PMPADDR0:
  case VX_CSR_MNSTATUS:
    break;
  default: {
      std::cout << std::hex << "Error: invalid CSR write addr=0x" << addr << ", value=0x" << value << std::endl;
      std::abort();
    }
  }
}

void Core::writeToStdOut(const void* data) {
  char c = *(char*)data;
  cout_buf_ << c;
  if (c == '\n') {
    std::cout << cout_buf_.str() << std::flush;
    cout_buf_.str("");
  }
}

void Core::cout_flush() {
  auto str = cout_buf_.str();
  if (!str.empty()) {
    std::cout << str << std::endl;
  }
}

bool Core::check_exit(Word* exitcode, bool riscv_test) const {
  if (exited_) {
    Word ec = reg_file_.at(3);
    if (riscv_test) {
      *exitcode = (1 - ec);
    } else {
      *exitcode = ec;
    }
    return true;
  }
  return false;
}

bool Core::running() const {
  return (perf_stats_.instrs != fetched_instrs_) || (fetched_instrs_ == 0);
}

void Core::attach_ram(RAM* ram) {
  mmu_.attach(*ram, 0, 0xFFFFFFFF);
}

void Core::showStats() {
  std::cout << std::dec << "PERF: instrs=" << perf_stats_.instrs << ", cycles=" << perf_stats_.cycles << std::endl;
}
