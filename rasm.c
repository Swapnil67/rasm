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
    fprintf(stdout, "Usage: ./rasm [file.rasm] [file.rm]\n");
}

int main(int argc, char *argv[]) {

    static Rm rm = {0};

    shift(&argc, &argv);

    // * Get the input .rasm file
    String_View input_filepath;
    if(argc > 0) {
	const char *input_file_str = shift(&argc, &argv);
	input_filepath = SV(input_file_str);
    }

    if(input_filepath.count == 0) {		
	fprintf(stderr, "Please provide a input rasm file\n");
	usage();
	exit(1);
    }

    // Get the output .rm file
    String_View output_filepath;
    if(argc > 0) {
	const char *output_file_str = shift(&argc, &argv);
	output_filepath = SV(output_file_str);
    }
    if(output_filepath.count == 0) {		
	fprintf(stderr, "Please provide a output file\n");
	usage();
	exit(1);
    }
    
    // * Converts rasm -> rm bytecode
    rasm_translate_source(&rm, input_filepath);

    // * saves rm bytecode to .rm file
    rasm_save_to_file(&rm, output_filepath);

    printf("Bytes of memory used: %ld\n", rm.arena_size);
    
    return 0;
}
