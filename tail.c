#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf [1024];

void tail (int fd, int num) {

  int temp;
  int linesTPrint;
  int emptyLine;
  int count = 0;
  int next,i;
  
  
  temp = open ("temp", O_CREATE | O_RDWR);
  
  
  while ((next = read(fd, buf, sizeof(buf))) > 0) {
    write (temp, buf, next);
    for (i = 0; i<next; i++) {
      if(buf[i] == '\n')
        emptyLine++;
    }
  }
  
  if (next < 0) {
    printf (1, "tail: read error \n");
    exit ();
  }
  if (emptyLine < num)
    linesTPrint = 0;
  linesTPrint = emptyLine - num;

  close (temp);
  temp = open ("temp", 0);

  while ((next = read(temp, buf, sizeof(buf))) > 0) {
    for ( i = 0; i<next; i++) {
      if (count >= linesTPrint)
        printf(1,"%c",buf[i]);
      if (buf[i] == '\n')
        count++;
      }
    }
  
  
    close (temp);
    unlink("temp");
}

int
main(int argc , char *argv[])
{
int fd;

if(argc <= 1)
{
   tail(0, 10);
}
if(argc == 2)
{
    if( argv[1][0] == '-')
    {
     int nextArg = atoi(++argv[1]);
     tail(0, nextArg);
    }
  
  
    else
    {
    if((fd = open(argv[1], 0)) < 0){
    printf(1, "tail: cannot open %s\n", argv[1]);
    }
    tail(fd, 10);
    }
}
if(argc > 2)
{
  
  
  
  if((fd = open(argv[2], 0)) < 0){
    printf(1, "tail: cannot open %s\n", argv[2]);
    exit();
  }
   int nextArg = atoi(++argv[1]);
   tail(fd, nextArg);
}

exit();
}
 

