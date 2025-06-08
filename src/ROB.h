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

#pragma once

#include <vector>
#include "instr.h"
#include "CDB.h"

namespace tinyrv {

class ReorderBuffer {
public:

  struct rob_entry_t {
    bool       valid;   // valid entry
    bool       ready;   // completed
    uint32_t   result;  // result data
    Instr::Ptr instr;   // instruction data
  };

  ReorderBuffer( uint32_t size);

  ~ReorderBuffer();

  bool full() const {
    return count_ == store_.size();
  }

  bool empty() const {
    return count_ == 0;
  }

  int allocate(Instr::Ptr instr);

  int pop();

  void update(const CommonDataBus::data_t& data);

  int head_index() const {
    return head_index_;
  }

  const rob_entry_t& get_entry(int index) const {
    return store_.at(index);
  }

  rob_entry_t& get_entry(int index) {
    return store_.at(index);
  }

  void dump();

private:

  std::vector<rob_entry_t> store_;
  int head_index_;
  int tail_index_;
  uint32_t count_;
};

}