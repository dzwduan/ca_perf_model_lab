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

#include "instr.h"

namespace tinyrv {

class Core;

class FunctionalUnit {
public:
  typedef std::shared_ptr<FunctionalUnit> Ptr;

  struct data_out_t {
    int      rob_index;
    int      rs_index;
    uint32_t result;
  };

  FunctionalUnit(uint32_t latency)
    : latency_(latency)
    , cycles_(0)
    , busy_(false)
    , done_(false)
  {}

  virtual ~FunctionalUnit() {}

  void execute() {
    if (!busy_ || done_)
      return;

    if (++cycles_ == latency_) {
      this->do_execute();
      done_ = true;
    }
  }

  bool busy() const {
    return busy_;
  }

  bool done() const {
    return done_;
  }

  data_out_t get_output() const {
    return {rob_index_, rs_index_, result_};
  }

  void issue(Instr::Ptr instr, int rob_index, int rs_index, uint32_t rs1_value, uint32_t rs2_value) {
    instr_     = instr;
    rob_index_ = rob_index;
    rs_index_  = rs_index;
    rs1_value_ = rs1_value;
    rs2_value_ = rs2_value;
    busy_      = true;
    done_      = false;
    cycles_    = 0;
  }

  void clear() {
    busy_ = false;
    done_ = false;
  }

protected:

  virtual void do_execute() = 0;

  Instr::Ptr instr_;
  uint32_t  rs1_value_;
  uint32_t  rs2_value_;
  uint32_t  result_;

private:

  int       rob_index_;
  int       rs_index_;

  uint32_t  latency_;
  uint32_t  cycles_;
  bool      busy_;
  bool      done_;
};

///////////////////////////////////////////////////////////////////////////////

class ALU : public FunctionalUnit {
public:
  ALU(Core* core)
    : FunctionalUnit(ALU_LATENCY)
    , core_(core)
  {}

  void do_execute();

private:
  Core* core_;
};

///////////////////////////////////////////////////////////////////////////////

class BRU : public FunctionalUnit {
public:
  BRU(Core* core)
    : FunctionalUnit(BRU_LATENCY)
    , core_(core)
  {}

  void do_execute();

private:
  Core* core_;
};

///////////////////////////////////////////////////////////////////////////////

class LSU : public FunctionalUnit {
public:
  LSU(Core* core)
    : FunctionalUnit(LSU_LATENCY)
    , core_(core)
  {}

  void do_execute();

private:
  Core* core_;
};

///////////////////////////////////////////////////////////////////////////////

class SFU : public FunctionalUnit {
public:
  SFU(Core* core)
    : FunctionalUnit(SFU_LATENCY)
    , core_(core)
  {}

  void do_execute();

private:
  Core* core_;
};

}