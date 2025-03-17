#define SV_IMPLEMENTATION
#define RM_IMPLEMENTATION

#include "./sv.h"
#include "./rasm.h"

static const char* shift(int *argc, char ***argv) {
    if(*argc < 0) return NULL;
    const char *arg = **argv;
    *argv += 1;
    *argc -= 1;
    return arg;
}

static void usage(void) {
    fprintf(stdout, "Usage: ./derasm [file.rm]\n");
}

static Rm rm = {0};

int main(int argc, char *argv[]) {
    shift(&argc, &argv);

    const char *filepath = shift(&argc, &argv);
    if(filepath == NULL) {
	fprintf(stderr, "ERROR: please provide a rasm bytecode file\n");
	usage();
	exit(1);
    }

    // Load the program into rm->program
    rm_load_program_from_file(&rm, filepath);
        
    // printf("Size: %ld\n", rm.rm_program_size);
    if(rm.rm_program_size <= 0) return 0;

    printf("main: \n");
    
    for(size_t i = 0; i < rm.rm_program_size; ++i) {
	printf("    %s", inst_as_cstr(rm.program[i].inst_type));
	if(inst_has_operand(rm.program[i].inst_type)) {
	    printf(" %ld", rm.program[i].inst_operand);
	}
	printf("\n");
    }
    
    return 0;
}
