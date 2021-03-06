#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "nist.h"

#define _32MB (32 * 1024 * 1024)
#define _08MB (8  * 1024 * 1024)
#define _04MB (4  * 1024 * 1024)
#define _02MB (2  * 1024 * 1024)


static int random_pool1 [_32MB];
char basename[FILENAME_MAX+1];

char *GetBaseDir(void)
{
  return basename;
}

int main(int argc, char **argv)
{
char *filename = "";
FILE *fp = stdin;
long result=0;

if (argc!=2) {
  printf("Usage sts <file>\n");
  return 1;
  }

/* Template file must be in current working directory */

(void)getcwd(basename,FILENAME_MAX);
strcat(basename, "/");

filename = argv[1];
if ((fp = fopen(filename, "rb")) == NULL) {
  printf("Cannot open file %s\n", filename);
  return 2;
  }
result = fread(random_pool1,sizeof(int),_04MB,fp);
fclose(fp);
if (result!=_04MB) {
  printf("16MB sample required %ld != %d\n",result, _04MB);
  return 3;
  }
if (PackTestF (random_pool1, _04MB, "nist.out") < 8) {
  if (PackTestL (random_pool1, _04MB, "nist.out") < 8)
    return 0;
  return 5;
  }
return 4;
}
