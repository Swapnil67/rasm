#define RM_IMPLEMENTATION
#include "./rasm.h"

static void load_program_from_memory(Rm *rm, Inst *program, size_t program_size) {
    assert(program_size < RM_PROGRAM_CAPACITY);
    memcpy(rm->program, program, sizeof(program[0]) * program_size);
    rm->rm_program_size = program_size;
}

static char *shift(int *argc, char ***argv) {
    assert(*argc > 0);
    char *arg = **argv;
    *argc -= 1;
    *argv += 1;
    return arg;
}

int main(int argc, char *argv[]) {

    Rm rm = {0};

    shift(&argc, &argv);

    int debug = 0, limit = 69;
    while(argc > 0) {
	char *arg = shift(&argc, &argv);
	if(strcmp(arg, "-d") == 0) {
	    debug = 1;
	}
	else if(strcmp(arg, "-l") == 0) {
	    char *endptr, *str = shift(&argc, &argv);
	    limit = (int) strtol(str, &endptr, 10);
	    if(endptr == str) {
		fprintf(stderr, "Invalid limit.\n");
		exit(EXIT_FAILURE);
	    }
	}

    }
    

    // printf("Debug mode %d\n", debug);

    Inst program[] = {
	MAKE_INST_PUSH(0),
	MAKE_INST_DUP(0),
	MAKE_INST_PUSH(1),
	MAKE_INST_PLUSI,
	MAKE_INST_DUP(0),
	MAKE_INST_JMP(2),
	MAKE_INST_HALT,	
    };

    printf("Program Size: %ld\n", sizeof(program[0]) * ARRAY_SIZE(program));
 

    // * Load the program from memory
    load_program_from_memory(&rm, program, ARRAY_SIZE(program));

    if(!debug) {
	// * Execute the program
	rm_execute_program(&rm, limit);

	// * Dump the stack
	rm_dump_stack(stdout, &rm);
    }
    else {
	while(limit != 0 && !rm.halt) {
	    // * Execute the instruction
	    Err err = rm_execute_inst(&rm);
	    if(err != ERR_OK) {
		fprintf(stderr, err_as_cstr(err));
		return err;
	    }
	    
	    if(limit > 0) {
		--limit;
	    }
	    
	    // * Dump the stack
	    rm_dump_stack(stdout, &rm);
	    getchar();	    
	}
    }

    return 0;
}
