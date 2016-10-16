#include <stdio.h>
// #include <ctype.h>
// #include <malloc.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
#include "parse_metafile.h"
//#define DEBUG
int main(int argc, char const *argv[]) {
  int restlt;

  // restlt = parse_metafile("fff.torrent");
  restlt = parse_metafile("YMDD-055.torrent");
  printf("read_metafile ok\n");

  return 0;
}
