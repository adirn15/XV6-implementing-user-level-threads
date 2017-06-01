// Simple grep.  Only supports ^ . * $ operators.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1,"!11\n");
  int x = alarm(60);
  x = alarm(0);
  printf(1,"x is %d\n",x);
  exit();
}