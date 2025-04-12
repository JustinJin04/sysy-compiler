#pragma once
#include <string>
#include <vector>
#include<iostream>
#include <string.h>
#include <cassert>

namespace KOOPA {

class RegPool{
 public:
  RegPool(int _size = 7): size(_size){
    reg_index_pool = new int[size];
    memset(reg_index_pool, 0, size * sizeof(int));
  }
  ~RegPool(){
    delete[] reg_index_pool;
  }

  std::string getReg(){
    for(int i=0;i<size;++i){
      if(reg_index_pool[i] == 0){
        reg_index_pool[i] = 1;
        return "t" + std::to_string(i);
      }
    }
    throw std::runtime_error("No available register");
  }

  std::string getReg(std::string reg){
    int ideal_idx = std::stoi(reg.substr(1));
    if(ideal_idx < 0 || ideal_idx >= size){
      throw std::runtime_error("Invalid register index");
    }
    if(reg_index_pool[ideal_idx] == 0){
      reg_index_pool[ideal_idx] = 1;
      return reg;
    } else {
      throw std::runtime_error("Register already in use");
    }
  }

  void freeReg(std::string reg){
    // assert(reg[0] == 't');
    int idx = std::stoi(reg.substr(1));
    if(idx < 0 || idx >= size){
      throw std::runtime_error("Invalid register index");
    }
    reg_index_pool[idx] = 0;
  }
  
 private:
  int* reg_index_pool;
  int size;
};




};  // namespace KOOPA