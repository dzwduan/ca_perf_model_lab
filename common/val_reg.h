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
#include <util.h>

namespace tinyrv {

template <typename T>
class ValReg : public SimObject<ValReg<T>> {
public:
  ValReg(const SimContext& ctx, const char* name, const T& init)
    : SimObject<ValReg<T>>(ctx, name)
    , init_(init)
    , data_(init)
    , data_next_(init)
  {}

  ~ValReg() {}

  const T& read() const {
    return data_;
  }

  void write(const T& data) {
    data_next_ = data;
  }

  void reset() {
    data_ = init_;
    data_next_ = init_;
  }

  void tick() {
    data_ = data_next_;
  }

protected:
  T init_;
  T data_;
  T data_next_;
};

}
