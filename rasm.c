#define RM_IMPLEMENTATION
#include "./rasm.h"

static void load_program_from_memory(Rm *rm, Inst *program, size_t program_size) {
    assert(program_size < RM_PROGRAM_CAPACITY);
    memcpy(rm->program, program, sizeof(program[0]) * program_size);
    rm->rm_program_size = program_size;
}


int main() {

    Rm rm = {0};

    Inst program[] = {
	MAKE_INST_PUSH(0),
	MAKE_INST_DUP(0),
	MAKE_INST_PUSH(1),
	MAKE_INST_PLUSI,
	MAKE_INST_JMP(2),
	MAKE_INST_HALT,	
    };

    printf("Program Size: %ld\n", sizeof(program[0]) * ARRAY_SIZE(program));
 

    // * Load the program from memory
    load_program_from_memory(&rm, program, ARRAY_SIZE(program));

    // * Execute the program
    int limit = 10;
    rm_execute_program(&rm, limit);

    // * Dump the stack
    rm_dump_stack(stdout, &rm);
    
    return 0;
}
