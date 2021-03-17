#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "quat.h"

#define VFDIM 2048
static char *velocity_field_file;

static struct option long_options[] = {
	{ "velocity-field",     required_argument, 0,  'v' },
	{ 0, 0, 0, 0 },
};

/* velocity field for 6 faces of a cubemap */
struct velocity_field {
        union vec3 v[6][VFDIM][VFDIM];
} *vf;

static void usage(char *msg)
{
	printf("ggvf_to_flowmap: %s\n", msg);
	printf("Usage: ggvf_to_flowmap -v velocity-field-file\n");
	exit(1);
}

static int restore_velocity_field(char *filename, struct velocity_field *vf)
{
	int fd;
	int bytes_left = (int) sizeof(*vf);
	int bytes_read = 0;

	if (!filename)
		return -1;

	printf("\n"); fflush(stdout);
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Cannot open '%s' for reading: %s\n",
			filename, strerror(errno));
		return -1;
	}

	/* FIXME: this is quick and dirty, not endian clean, etc. */
	do {
		unsigned char *x = (unsigned char *) vf;
		int rc = read(fd, &x[bytes_read], bytes_left);
		if (rc < 0 && errno == EINTR)
			continue;
		if (rc < 0) {
			fprintf(stderr, "Error reading '%s': %s\n", filename, strerror(errno));
			fprintf(stderr, "Velocity field restoration failed.\n");
			close(fd);
			return -1;
		}
		bytes_read += rc;
		bytes_left -= rc;
	} while (bytes_left > 0);
	close(fd);
	printf("Velocity field restored from %s\n", filename);
	return 0;
}

int main(int argc, char *argv[])
{
	int rc;

	/* Allocate a ton of memory. We allocate these rather than
	 * declaring them statically because if DIM is too large, the
	 * static data will not fit into the .bss (see https://en.wikipedia.org/wiki/.bss)
	 */
	vf = calloc(1, sizeof(struct velocity_field));

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

	if (!velocity_field_file)
		usage("velocity_field_file required");

	rc = restore_velocity_field(velocity_field_file, vf);
	if (rc)
		return rc;

	return 0;	
}

