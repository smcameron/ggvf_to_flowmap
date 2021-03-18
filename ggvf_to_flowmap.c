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
#include "mesh.h"

#define VFDIM 2048
#define SPHERE_SUBDIVISIONS 64
static char *velocity_field_file;
static char *flowmap_file;

static struct option long_options[] = {
	{ "velocity-field",     required_argument, 0,  'v' },
	{ "flow-map",     required_argument, 0,  'f' },
	{ 0, 0, 0, 0 },
};

struct flowmap {
        union vec2 v[6][VFDIM][VFDIM];
} *fm;

/* velocity field for 6 faces of a cubemap */
struct velocity_field {
        union vec3 v[6][VFDIM][VFDIM];
} *vf;

struct mesh *sphere;

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

/* convert from cubemap coords to cartesian coords on surface of sphere */
static union vec3 fij_to_xyz(int f, int i, int j, const int dim)
{
	union vec3 answer;

	switch (f) {
	case 0:
		answer.v.x = (float) (i - dim / 2) / (float) dim;
		answer.v.y = -(float) (j - dim / 2) / (float) dim;
		answer.v.z = 0.5;
		break;
	case 1:
		answer.v.x = 0.5;
		answer.v.y = -(float) (j - dim / 2) / (float) dim;
		answer.v.z = -(float) (i - dim / 2) / (float) dim;
		break;
	case 2:
		answer.v.x = -(float) (i - dim / 2) / (float) dim;
		answer.v.y = -(float) (j - dim / 2) / (float) dim;
		answer.v.z = -0.5;
		break;
	case 3:
		answer.v.x = -0.5;
		answer.v.y = -(float) (j - dim / 2) / (float) dim;
		answer.v.z = (float) (i - dim / 2) / (float) dim;
		break;
	case 4:
		answer.v.x = (float) (i - dim / 2) / (float) dim;
		answer.v.y = 0.5;
		answer.v.z = (float) (j - dim / 2) / (float) dim;
		break;
	case 5:
		answer.v.x = (float) (i - dim / 2) / (float) dim;
		answer.v.y = -0.5;
		answer.v.z = -(float) (j - dim / 2) / (float) dim;
		break;
	}
	vec3_normalize_self(&answer);
	return answer;
}


static void transform_world_vector_to_tangent_space(union vec3 *world_space_vec,
		union vec3 *tangent, union vec3 *bitangent, union vec3 *normal,
		union vec2 *tangent_space_output)
{
	/* TODO: implement this */
}

static void render_flowmap_to_png(struct flowmap *fm, char *flowmap_file)
{
	/* TODO: implement this */
}

int main(int argc, char *argv[])
{
	int rc, f, i, j;

	while (1) {

		int option_index = 0;
		int rc;

		rc = getopt_long(argc, argv, "f:v:", long_options, &option_index);
		if (rc == -1)
			break;

		switch (rc) {
		case 'f':
			flowmap_file = optarg;
			break;
		case 'v':
			velocity_field_file = optarg;
			break;
		}
	}

	if (!velocity_field_file)
		usage("velocity_field_file required");

	/* Allocate a ton of memory. We allocate these rather than
	 * declaring them statically because if DIM is too large, the
	 * static data will not fit into the .bss (see https://en.wikipedia.org/wiki/.bss)
	 */
	vf = calloc(1, sizeof(*vf));
	fm = calloc(1, sizeof(*fm));

	rc = restore_velocity_field(velocity_field_file, vf);
	if (rc)
		return rc;

	sphere = mesh_unit_spherified_cube(SPHERE_SUBDIVISIONS);

	/* Transform the velocity field into a tangent space flowmap */
	for (f = 0; f < 6; f++) {
		for (i = 0 ; i < VFDIM; i++) {
			for (j = 0; j < VFDIM; j++) {
				union vec3 tangent, bitangent, normal;
				union vec2 output;
				union vec3 position = fij_to_xyz(f, i, j, VFDIM);
				int vindex = mesh_find_nearest_cube_vertex_on_face(sphere, f, SPHERE_SUBDIVISIONS, &position);
				mesh_get_tbn_from_vertex(sphere, vindex, &tangent, &bitangent, &normal);
				transform_world_vector_to_tangent_space(&vf->v[f][i][j], &tangent, &bitangent, &normal, &output);
				fm->v[f][i][j] = output;
			}
		}
	}

	render_flowmap_to_png(fm, flowmap_file);

	return 0;	
}

