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

#include <memory>
#include <iostream>
#include <queue>
#include <util.h>

namespace tinyrv {

template <typename T>
class FiFoReg : public SimObject<FiFoReg<T>> {
public:
  FiFoReg(const SimContext& ctx, const char* name, uint32_t depth = 1)
    : SimObject<FiFoReg<T>>(ctx, name)
    , depth_(depth)
    , push_pending_(false)
    , pop_pending_(false)
  {
    assert(depth > 0);
  }

  ~FiFoReg() {}

  bool empty() const {
    return (buffer_.empty() && !push_pending_)
        || (buffer_.size() == 1 && pop_pending_);
  }

  bool full() const {
    return (buffer_.size() == depth_ && !pop_pending_)
        || (buffer_.size() == (depth_-1) && push_pending_);
  }

  const T& data() const {
    assert(!buffer_.empty());
    return buffer_.front();
  }

  void push(const T& data) {
    assert(!full());
    push_pending_ = true;
    push_data_ = data;
  }

  void pop() {
    assert(!empty());
    pop_pending_ = true;
  }

  void reset() {
    buffer_ = std::queue<T>();
    push_pending_ = false;
    pop_pending_ = false;
  }

  void tick() {
    if (pop_pending_ && !buffer_.empty()) {
      buffer_.pop();
      pop_pending_ = false;
    }
    if (push_pending_ && buffer_.size() < depth_) {
      buffer_.push(push_data_);
      push_pending_ = false;
    }
  }

protected:
  uint32_t depth_;
  std::queue<T> buffer_;
  bool push_pending_;
  bool pop_pending_;
  T push_data_;
};

}
