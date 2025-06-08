// Copyright 2024 blaise
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>
#include "instr.h"
#include "CDB.h"

namespace tinyrv {

class ReservationStation {
public:

  struct entry_t {
    bool valid;       // valid entry
    bool running;     // has been assigned an FU
    int rob_index;    // allocated ROB index
    int rs1_index;    // RS producing rs1 (-1 indicates data is already available)
    int rs2_index;    // RS producing rs2 (-1 indicates data is already available)
    uint32_t rs1_data; // rs1 data
    uint32_t rs2_data; // rs2 data
    uint32_t barrier_id; // barrier id to enforce ordering fo LSU instructions
    Instr::Ptr instr; // instruction data

    bool operands_ready() const {
      return rs1_index == -1 && rs2_index == -1;
    }

    void update_operands(const CommonDataBus::data_t& data) {
      // update operands if this RS entry is waiting for them
      // TODO:
    }
  };

  ReservationStation(uint32_t size);

  ~ReservationStation();

  bool operands_ready(uint32_t index) const {
    // are all operands ready?
    // TODO:
  }

  const entry_t& get_entry(uint32_t index) const {
    return store_.at(index);
  }

  entry_t& get_entry(uint32_t index) {
    return store_.at(index);
  }

  int issue(int rob_index, int rs1_index, int rs2_index, uint32_t rs1_data, uint32_t rs2_data, Instr::Ptr instr);

  void release(uint32_t index);

  bool locked(uint32_t index) const;

  bool full() const {
    return (next_index_ == store_.size());
  }

  bool empty() const {
    return (next_index_ == 0);
  }

  uint32_t size() const {
    return store_.size();
  }

  void dump() {
    for (uint32_t i = 0; i < store_.size(); ++i) {
      auto& entry = store_[i];
      if (entry.valid) {
        DT(4, "RS[" << i << "] rob=" << entry.rob_index << ", running=" << entry.running << ", rs1=" << entry.rs1_index << ", rs2=" << entry.rs2_index << " (#" << entry.instr->getId() << ")");
      }
    }
  }

private:

  std::vector<entry_t>  store_;
  std::vector<uint32_t> indices_;
  uint32_t next_index_;
  uint32_t lsu_barrier_tick_;
  uint32_t lsu_barrier_tock_;
  TicketBarrier lsu_barrier_;
};

}