#pragma once

#include <koopa.h>
#include "visitor.hpp"
#include <vector>
#include <cassert>

namespace KOOPA {

class AggregateVisitor : public Visitor {
 public:
  std::vector<int> init_values;

  void visit(const koopa_raw_aggregate_t& agg) {
    for(int i=0;i<agg.elems.len;++i){
      auto ptr = reinterpret_cast<koopa_raw_value_t>(agg.elems.buffer[i]);
      if(ptr->kind.tag == KOOPA_RVT_INTEGER){
        init_values.push_back(ptr->kind.data.integer.value);
      } else if (ptr->kind.tag == KOOPA_RVT_AGGREGATE) {
        visit(ptr->kind.data.aggregate);
      } else {
        assert(0);
      }
    }
  }





};


















}; // namespace KOOPA