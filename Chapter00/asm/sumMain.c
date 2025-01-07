#include <stdio.h>

extern int calculate_sum(int n);

int main() {
  int n = 10;
  int sum = calculate_sum(n);

  printf("The sum of numbers from 1 to %d is %d\n", n, sum);
  return 0;
}
