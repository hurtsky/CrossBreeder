#include <stdio.h>

FILE *fptr_WRITE_WAKEUP_THRESHOLD;
char one;
main() {
fptr_WRITE_WAKEUP_THRESHOLD=fopen("/sys/power/wait_for_fb_sleep","r");
  if ( fptr_WRITE_WAKEUP_THRESHOLD != NULL ) {
    fscanf(fptr_WRITE_WAKEUP_THRESHOLD,"%c",&one);
  }
  fclose(fptr_WRITE_WAKEUP_THRESHOLD);
printf("%c\n",one);
}
