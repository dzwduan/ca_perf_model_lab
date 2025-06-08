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

#include <simobject.h>
#include "instr.h"

namespace tinyrv {

class CommonDataBus {
public:

  struct data_t {
    uint32_t result;
    int      rob_index;
    int      rs_index;
  };

  CommonDataBus() : empty_(true) {}

  ~CommonDataBus() {}

  bool empty() const {
    return empty_;
  }

  const data_t& data() const {
    return data_;
  }

  void push(uint32_t result, int rob_index, int rs_index) {
    data_ = {result, rob_index, rs_index};
    empty_ = false;
  }

  void pop() {
    empty_ = true;
  }

private:
  bool   empty_;
  data_t data_;
};

}