#pragma once

#include <koopa.h>
#include <cassert>

namespace KOOPA {

inline int get_type_width(const koopa_raw_type_t& type){
  if(type->tag == KOOPA_RTT_INT32 || type->tag == KOOPA_RTT_POINTER) {
    return 4;
  } else if (type->tag == KOOPA_RTT_ARRAY) {
    return get_type_width(type->data.array.base) * type->data.array.len;
  } else {
    assert(0);
  }
}

}; // namespace KOOPA
