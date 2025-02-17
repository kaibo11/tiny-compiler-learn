#pragma once
#include <iostream>
#include <vector>

#include "StackElement.hpp"

class Stack {
private:
  std::vector<StackElement> elements;

public:
  void push(const StackElement &value) {
    elements.push_back(value);
  }

  void pop() {
    if (!elements.empty()) {
      return elements.pop_back();
    } else {
      std::cerr << "empty stack, can't pop." << std::endl;
    }
  }

  StackElement top() const {
    if (!elements.empty()) {
      return elements.back();
    } else {
      throw std::out_of_range("empty stack, can't visit stack element.");
    }
  }

  bool empty() const {
    return elements.empty();
  }

  std::size_t size() const {
    return elements.size();
  }
};