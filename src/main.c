#include <share/array.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int* array = array(int);
	array_push(array, 10);

	printf("%zu\n", array_size(array));

	array_free(array);
	return 0;
}
