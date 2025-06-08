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
#include "types.h"
#include "debug.h"
#include "RS.h"

using namespace tinyrv;

ReservationStation::ReservationStation(uint32_t size)
  : store_(size)
  , indices_(size)
  , next_index_(0) {
  for (uint32_t i = 0; i < size; ++i) {
    store_[i].valid = false;
    indices_[i] = i;
  }
}

ReservationStation::~ReservationStation() {}

int ReservationStation::issue(int rob_index, int rs1_index, int rs2_index, uint32_t rs1_data, uint32_t rs2_data, Instr::Ptr instr) {
    assert(!this->full());
    int index = indices_[next_index_++];
    uint32_t barrier_id = 0;
    if (instr->getFUType() == FUType::LSU) {
      barrier_id = lsu_barrier_.tick();
    }
    store_[index] = {true, false, rob_index, rs1_index, rs2_index, rs1_data, rs2_data, barrier_id, instr};
    assert(index != rs1_index);
    assert(index != rs2_index);
    return index;
  }

  void ReservationStation::release(uint32_t index) {
    assert(!this->empty());
    auto& entry = store_.at(index);
    entry.valid = false;
    entry.running = false;
    if (entry.instr->getFUType() == FUType::LSU) {
      lsu_barrier_.tock();
    }
    indices_[--next_index_] = index;
  }

  bool ReservationStation::locked(uint32_t index) const {
    auto& entry = store_.at(index);
    if (!entry.valid || entry.instr->getFUType() != FUType::LSU)
      return false;
    return !lsu_barrier_.ready(entry.barrier_id);
  }
