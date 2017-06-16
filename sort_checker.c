#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	int sorted = 1, retval;
	char *buff;
	size_t len;
	unsigned long num, prev_num, num_vals = 1;

	if (argc != 2) {
		printf("Usage: sort_checker <sorted_file>");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		perror("Failed to open the file");
		return errno;
	}

	while (!feof(fp)) {
		getline(&buff, &len, fp);
		retval = sscanf(buff, "%lu", &prev_num);		
		if (retval == 1)
			break;
	}		

	while (!feof(fp)) {
		getline(&buff, &len, fp);
		sscanf(buff, "%lu", &num);

		if (num < prev_num)
			sorted = 0;

		prev_num = num;
		num_vals++;
	}

	printf("Total numbers: %lu\n", num_vals);

	if (sorted)
		printf("The numbers are sorted\n");
	else
		printf("The numbers are not sorted\n");

	return 0;
}
