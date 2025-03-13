#define SV_IMPLEMENTATION
#define RM_IMPLEMENTATION
#include "./sv.h"
#include "./rasm.h"

// static void load_program_from_memory(Rm *rm, Inst *program, size_t program_size) {
//     assert(program_size < RM_PROGRAM_CAPACITY);
//     memcpy(rm->program, program, sizeof(program[0]) * program_size);
//     rm->rm_program_size = program_size;
// }

static char *shift(int *argc, char ***argv) {
    // assert(*argc > 0);
    if(*argc <= 0) return NULL;
    char *arg = **argv;
    *argc -= 1;
    *argv += 1;
    return arg;
}

static void usage(void) {
    fprintf(stdout, "Usage: ./rasm -p [file.rasm]\n");
}

int main(int argc, char *argv[]) {

    static Rm rm = {0};

    shift(&argc, &argv);

    String_View input_filepath;

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
	else if(strcmp(arg, "-p") == 0) {
	    const char *str = shift(&argc, &argv);
	    if(str == NULL) {		
		fprintf(stderr, "Please provide a rasm file\n");
		usage();
		exit(1);
	    }
	    input_filepath = SV(str);
	}
    }

    // Inst program[] = {
    // 	MAKE_INST_PUSH(0),
    // 	MAKE_INST_DUP(0),
    // 	MAKE_INST_PUSH(1),
    // 	MAKE_INST_PLUSI,
    // 	MAKE_INST_DUP(0),
    // 	MAKE_INST_JMP(2),
    // 	MAKE_INST_HALT,	
    // };

    // * Load the program from memory
    // load_program_from_memory(&rm, program, ARRAY_SIZE(program));

    // * Load the program from file
    String_View original_source = rm_load_program_from_file(&rm, input_filepath);

    rasm_translate_source(&rm, original_source);
        
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
