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

#pragma once

#include <string>
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <unordered_map>
#include <sstream>
#include <memory>
#include <set>
#include <simobject.h>
#include <mem.h>
#include "debug.h"
#include "types.h"
#include "val_reg.h"
#include "fifo_reg.h"
#include "instr.h"
#include "RAT.h"
#include "RS.h"
#include "RST.h"
#include "ROB.h"
#include "FU.h"
#include "CDB.h"

namespace tinyrv {

class ProcessorImpl;
class Instr;
class RAM;

class Core : public SimObject<Core> {
public:
  struct PerfStats {
    uint64_t cycles;
    uint64_t instrs;

    PerfStats()
      : cycles(0)
      , instrs(0)
    {}
  };

  Core(const SimContext& ctx, uint32_t core_id, ProcessorImpl* processor);
  ~Core();

  void reset();

  void tick();

  void attach_ram(RAM* ram);

  bool running() const;

  bool check_exit(Word* exitcode, bool riscv_test) const;

  void showStats();

private:

  Instr::Ptr decode(uint32_t instr_code, uint32_t PC, uint64_t uuid) const;

  void dmem_read(void* data, uint64_t addr, uint32_t size);

  void dmem_write(const void* data, uint64_t addr, uint32_t size);

  void set_csr(uint32_t addr, uint32_t value);

  uint32_t get_csr(uint32_t addr);

  void writeToStdOut(const void* data);

  void cout_flush();

  struct id_data_t {
    uint32_t instr_code;
    Word     PC;
    uint64_t uuid;
  };

  struct is_data_t {
    Instr::Ptr instr;
  };

  struct ex_data_t {
    Instr::Ptr instr;
    uint32_t rs1_data;
    uint32_t rs2_data;
  };

  void fetch();
  void decode();
  void issue();
  void execute();
  void writeback();
  void commit();

  uint32_t core_id_;
  ProcessorImpl* processor_;
  MemoryUnit mmu_;

  std::vector<Word> reg_file_;
  Word PC_;

  FiFoReg<id_data_t>::Ptr decode_queue_;
  FiFoReg<is_data_t>::Ptr issue_queue_;
  ValReg<bool>::Ptr fetch_stalled_;

  ReorderBuffer       ROB_;
  RegisterAliasTable  RAT_;
  ReservationStation  RS_;
  RegisterStatusTable RST_;
  CommonDataBus       CDB_;
  std::vector<FunctionalUnit::Ptr> FUs_;
  bool exited_;

  std::stringstream cout_buf_;

  uint64_t uuid_ctr_;

  PerfStats perf_stats_;
  uint64_t fetched_instrs_;

  friend class ALU;
  friend class BRU;
  friend class LSU;
  friend class SFU;
};

} // namespace tinyrv
