#include <string>
#include "util.hpp"
uint64_t convertStringToUint64(const std::string& str) {
    bool isNegative = false;
    std::string numStr = str;
  
    // 检查是否为负数
    if (str[0] == '-') {
        isNegative = true;
        numStr = str.substr(1); // 去掉负号
    }
  
    // 将字符串转换为无符号整数
    uint64_t num = std::stoull(numStr);
  
    // 如果是负数，计算补码
    if (isNegative) {
        num = ~num + 1; // 取反加1
    }
  
    return num;
  }