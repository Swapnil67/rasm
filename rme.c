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
    fprintf(stdout, "Usage: ./rme -i [file.rm]\n");
}

static Rm rm = {0};

int main(int argc, char *argv[]) {
    shift(&argc, &argv);

    bool debug = false;
    int limit = 69;
    const char *input_file = NULL;
    
    while(argc > 0) {
	const char *arg = shift(&argc, &argv);
	if(strcmp(arg, "-i") == 0) {
	    input_file = shift(&argc, &argv);
	}
	else if(strcmp(arg, "-d") == 0) {
	    debug = true;
	}
    }

    if(input_file == NULL) {
	fprintf(stderr, "ERROR: please provide input file\n");
	usage();
	exit(1);
    }

    // Load the program into rm->program
    rm_load_program_from_file(&rm, input_file);
        
    if(!debug) {
	// * execute the program
	rm_execute_program(&rm, limit);

	// * dump the stack
	rm_dump_stack(stdout, &rm);
    }
    else {
	while(limit != 0 && !rm.halt) {
	    // * execute the instruction
	    Err err = rm_execute_inst(&rm);
	    if(err != ERR_OK) {
		fprintf(stderr, err_as_cstr(err));
		return err;
	    }
	    
	    if(limit > 0) {
		--limit;
	    }
	    
	    // * dump the stack
	    rm_dump_stack(stdout, &rm);
	    getchar();	    
	}
    }

    return 0;
}
