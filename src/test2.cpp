// This is a simple test program

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include "vmms.h"

struct student
{
  int	ID;
  char	name[20];
  struct student *left;
  struct student *right;
};

int main(int argc, char** argv)
{
	int rc = 0;
	char *student_ptr;
	char *list;


	list = (char*) vmms_malloc(50, &rc);

	if (list == NULL)
		return 1;
	/*
	list->right = list->left = NULL;
	strcpy (list->name, "dummy1");
	list->ID = 911;

	printf("Name = %s; ID = %d\n", list->name, list->ID);
	*/

	printf("list address = %8x; Name = %s; ID = %s\n", list, list, (char*)list+10);

	system("pause");

	rc = vmms_free((char*)list);

  return rc;
}



	


