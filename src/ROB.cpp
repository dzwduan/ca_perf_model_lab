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
#include "ROB.h"

using namespace tinyrv;

ReorderBuffer::ReorderBuffer(uint32_t size)
  : store_(size)
  , head_index_(0)
  , tail_index_(0)
  , count_(0)
{
  for (auto& entry : store_) {
    entry.valid = false;
    entry.ready = false;
  }
}

ReorderBuffer::~ReorderBuffer() {
  //--
}

int ReorderBuffer::allocate(Instr::Ptr instr) {
  assert(!this->full());
  int index = tail_index_;
  store_[index] = {true, false, 0, instr};
  tail_index_ = (tail_index_ + 1) % store_.size();
  ++count_;
  return index;
}

void ReorderBuffer::update(const CommonDataBus::data_t& data) {
  auto& entry = store_[data.rob_index];
  assert(entry.valid);
  assert(!entry.ready);

  // Udate the ROB entry
  // TODO:

  if (entry.instr->getExeFlags().use_rd) {
    DT(2, "Writeback: value=0x" << std::hex << data.result << std::dec << ", " << *entry.instr);
  } else {
    DT(2, "Writeback: " << *entry.instr);
  }
}

int ReorderBuffer::pop() {
  assert(!this->empty());
  assert(store_[head_index_].valid);
  assert(store_[head_index_].ready);
  store_[head_index_].valid = false;
  store_[head_index_].ready = false;
  head_index_ = (head_index_ + 1) % store_.size();
  --count_;
  return head_index_;
}

void ReorderBuffer::dump() {
  for (int i = 0; i < (int)store_.size(); ++i) {
    auto& entry = store_[i];
    if (entry.valid) {
      DT(4, "ROB[" << i << "] ready=" << entry.ready << ", head=" << (i == head_index_) << " (#" << entry.instr->getId() << ")");
    }
  }
}