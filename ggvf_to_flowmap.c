#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

static char *velocity_field_file;

static struct option long_options[] = {
	{ "velocity-field",     required_argument, 0,  'v' },
	{ 0, 0, 0, 0 },
};

int main(int argc, char *argv[])
{

	while (1) {

		int option_index = 0;
		int rc;

		rc = getopt_long(argc, argv, "v:", long_options, &option_index);
		if (rc == -1)
			break;

		switch (rc) {
		case 'v':
			velocity_field_file = optarg;
			break;
		}
	}

	return 0;	
}

