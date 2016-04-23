/*	Elton Vinh
	Project 1
	CS149
	Frank Butt
*/
// mmc.cpp - memory console driver

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "vmms.h"

int main(int argc, char** argv)
{
  char command[32], filename[30];
  int byte_boundary;
  int rc = 0;
	
  if (argc != 2)
  {
    printf("Usage: %s boundary_size\n", argv[0]);
    return 1;
  }

  byte_boundary = atoi(argv[1]);
  rc = mmc_initialize ( byte_boundary );
  
  printf("Memory Management Console\n");
 
  while(1)
  {
    printf("Enter Command: D/M/E ---> ");
	fgets(command, sizeof(command), stdin);

    if ((command[0] == 'D') || (command[0] == 'd'))
    {
      if (strlen(command)<= 2)
        rc = mmc_display_memory("");
      else
        rc = mmc_display_memory((char*)command+2);
    }
    else if ((command[0] == 'M') || (command[0] == 'm'))
    {
      if (strlen(command)<= 2)
        rc = mmc_display_memtable("");
      else
        rc = mmc_display_memtable((char*)command+2);
    }
    else if ((command[0] == 'E') || (command[0] == 'e'))
      return rc;
  }
}



	


