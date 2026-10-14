#include <assert.h>
#include <stdio.h>

#include "add.h"

int main(void) {
  assert(add(0, 0) == 0);
  assert(add(2, 3) == 5);
  assert(add(-4, 4) == 0);
  assert(add(-2, -3) == -5);

  printf("add_test: ok\n");
  return 0;
}
