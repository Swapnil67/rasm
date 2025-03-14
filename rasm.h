#ifndef RM_H_
#define RM_H_

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#if defined(__GNUC__) || defined(__clang__)
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#  error "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/tsoding/bng"
#endif

#define RM_STACK_CAPACITY 1024
#define RM_PROGRAM_CAPACITY 1024
#define RM_ARENA_CAPACITY  (10 * 1000 * 1000)

#define ARRAY_SIZE(arr) sizeof(arr)/sizeof(arr[0])

#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int)sv.count, sv.data

#define RASM_COMMENT_SYMBOL ';'
#define RASM_PP_SYMBOL '%'

typedef enum {
    INST_NOP,
    INST_HALT,

    INST_PUSH,
    INST_DUP,
    INST_JMP,

    INST_PLUSI,
    INST_MINUSI,
    INST_MULI,
    INST_DIVI,
    INST_MODI,
} Inst_Type;

typedef struct {
    Inst_Type  inst_type;
    uint64_t   inst_operand;
} Inst;

const char* inst_as_cstr(Inst_Type type);
const char* inst_to_cstr(Inst_Type type);
bool inst_has_operand(Inst_Type type);

// * vm error's
typedef enum {
    ERR_STACK_UNDERFLOW = 0,
    ERR_STACK_OVERFLOW,
    ERR_ILLEGAL_INST,
    ERR_OK,
} Err;
const char* err_as_cstr(Err err);

typedef struct {
    int64_t stack[RM_STACK_CAPACITY];    
    uint64_t rm_stack_size;
    
    Inst program[RM_PROGRAM_CAPACITY];
    uint64_t rm_program_size;
    uint64_t ip;

    char arena[RM_ARENA_CAPACITY];
    size_t arena_size;

    bool halt;
} Rm;

void *arena_sv_to_cstr(Rm *rm, String_View sv);
void *arena_alloc(Rm *rm, size_t n);
String_View arena_slurp_file(Rm *rm, String_View filepath);
void rasm_translate_source(Rm *rm, String_View original_source);
void rasm_save_to_file(Rm *rm, String_View filepath);

void rm_dump_stack(FILE *stream, Rm *rm);
void rm_load_program_from_file(Rm *rm, const char* filepath);
Err rm_execute_program(Rm *rm, int limit);
Err rm_execute_inst(Rm *rm);

#define RM_FILE_MAGIC 0x4D42

PACK(struct Rm_File_Meta {
    uint16_t magic;
    uint64_t program_size;
});

typedef struct Rm_File_Meta Rm_File_Meta;

#endif // RM_H_

#ifdef RM_IMPLEMENTATION


const char* err_as_cstr(Err err) {
    switch(err) {
    case ERR_OK: return "ERR_OK";
    case ERR_ILLEGAL_INST: return "ERR_ILLEGAL_INST";
    case ERR_STACK_OVERFLOW: return "ERR_STACK_OVERFLOW";
    case ERR_STACK_UNDERFLOW: return "ERR_STACK_UNDERFLOW";
    default:
	return "Unknown Err";
    }
}

const char* inst_to_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP: return "INST_NOP";
    case INST_HALT: return "INST_HALT";
    case INST_PUSH: return "INST_PUSH";
    case INST_DUP: return "INST_DUP";
    case INST_JMP: return "INST_JMP";
    
    case INST_PLUSI: return "INST_PLUSI";
    case INST_MINUSI: return "INST_MINUSI";
    case INST_MULI: return "INST_MULI";
    case INST_DIVI: return "INST_DIVI";
    case INST_MODI: return "INST_MODI";
default:
    return "Unknown type";
    }
}

const char* inst_as_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP: return "nop";
    case INST_HALT: return "halt";
    case INST_PUSH: return "push";
    case INST_DUP: return "dup";
    case INST_JMP: return "jmp";
    
    case INST_PLUSI: return "plusi";
    case INST_MINUSI: return "minusi";
    case INST_MULI: return "muli";
    case INST_DIVI: return "divi";
    case INST_MODI: return "modi";
default:
    return "Unknown type";
    }
}

bool inst_has_operand(Inst_Type type) {
    switch(type) {
    case INST_NOP: return false;
    case INST_HALT: return false;
    case INST_PUSH: return true;
    case INST_DUP: return true;
    case INST_JMP: return true;
    
    case INST_PLUSI: return false;
    case INST_MINUSI: return false;
    case INST_MULI: return false;
    case INST_DIVI: return false;
    case INST_MODI: return false;
default:
    fprintf(stderr, "ERROR: unknown Inst_Type\n");
    exit(1);
    }
}

void *arena_sv_to_cstr(Rm *rm, String_View sv) {
    assert(rm->arena_size + (sv.count + 1) < RM_ARENA_CAPACITY);
    void *result = rm->arena + rm->arena_size; 
    memcpy(result, sv.data, sv.count);
    rm->arena_size += (sv.count + 1);
    //result[sv.count] = '\0';
    return result;
}

void *arena_alloc(Rm *rm, size_t n) {
    assert(rm->arena_size + n < RM_ARENA_CAPACITY);
    void *result = rm->arena + rm->arena_size;
    rm->arena_size += n;
    return result;
}

String_View arena_slurp_file(Rm *rm, String_View filepath) {
    const char *filepath_cstr = arena_sv_to_cstr(rm, filepath);

    FILE *file_fd = fopen(filepath_cstr, "r");
    if(file_fd == NULL) {
	fprintf(stderr, "ERROR: Could Not read File %s\n", strerror(errno));
	exit(1);
    }

    if(fseek(file_fd, 0, SEEK_END) < 0) {
	fprintf(stderr, "ERROR: Could Not read File %s\n", strerror(errno));
	exit(1);
    }

    long m = ftell(file_fd);
    if(m < 0) {
	fprintf(stderr, "ERROR: Could Not read File %s\n", strerror(errno));
	exit(1);
    }

    // * Allocate buffer of m size
    void *buffer = arena_alloc(rm, (size_t)m);
    if(buffer == NULL) {
	fprintf(stderr, "ERROR: Could Not allocate memory for the file %s\n", strerror(errno));
	exit(1);
    }

    // * Set file pos at the start of file
    if(fseek(file_fd, 0, SEEK_SET) < 0) {
	fprintf(stderr, "ERROR: Could Not read File %s\n", strerror(errno));
	exit(1);
    }

    // Copy from 0 - m into buffer
    size_t n = fread(buffer, 1, (size_t)m, file_fd);
    if(ferror(file_fd)) {
	fprintf(stderr, "ERROR: Could Not read File %s\n", strerror(errno));
	exit(1);
    }
    
    fclose(file_fd);

    return (String_View) { .count = n, .data = buffer }; 
}

// * Translate RM program from Text To Binary (create .rm bytecode executables)
void rasm_translate_source(Rm *rm, String_View input_filepath) {
    // * Load the program from file
    String_View original_source = arena_slurp_file(rm, input_filepath);

    int line_number = 0;

    while(original_source.count > 0) {
	String_View line = sv_chop_by_delim(&original_source, '\n');
	// printf("Line: "SV_Fmt"\n", SV_Arg(line));
	
	line_number += 1;
	// Check if comment
	if(*line.data == RASM_COMMENT_SYMBOL) {
	    continue;
	}

	String_View token = sv_chop_by_delim(&line, ' ');
	// printf("Token: "SV_Fmt"\n", SV_Arg(token));
	
	if(token.count > 0 && *token.data == RASM_PP_SYMBOL) {
	    // TODO Check for pre-processors
	}

	// TODO Check for labels

	// Instructions
	if(token.count > 0) {
	    // * Get the operand
	    String_View operand = sv_trim(sv_chop_by_delim(&line, RASM_COMMENT_SYMBOL));
	    if(sv_eq(token, SV(inst_as_cstr(INST_PUSH)))) {
		Inst_Type inst_type = INST_PUSH;
		rm->program[rm->rm_program_size].inst_type = INST_PUSH;
		
		if(inst_has_operand(inst_type)) {
		    char *str = arena_sv_to_cstr(rm, operand);
		    char *endptr;
		    int64_t val = strtoll(str, &endptr, 10);
		    if(endptr == str) {
			fprintf(stderr, "No digits were found\n");
			exit(1);
		    }
		    rm->program[rm->rm_program_size].inst_operand = (uint64_t) val;
		    rm->rm_program_size += 1;
		}
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_PLUSI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_PLUSI;
		rm->rm_program_size += 1;		
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_MINUSI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_MINUSI;
		rm->rm_program_size += 1;		
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_MULI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_MULI;
		rm->rm_program_size += 1;		
	    }	    	       	    
	    else if(sv_eq(token, SV(inst_as_cstr(INST_DIVI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_DIVI;
		rm->rm_program_size += 1;		
	    }	    	       	    
	    else if(sv_eq(token, SV(inst_as_cstr(INST_HALT)))) {
		rm->program[rm->rm_program_size].inst_type = INST_HALT;
		rm->rm_program_size += 1;
	    }
	    else {
		fprintf(stderr, ""SV_Fmt":%d: ERROR unknown instruction `"SV_Fmt"`\n",
		SV_Arg(input_filepath), line_number, SV_Arg(token));
		exit(1);
	    }
	    
	}
    }
}

void rm_dump_stack(FILE *stream, Rm *rm) {
    fprintf(stream, "Stack:\n");
    if(rm->rm_stack_size > 0) {
	for(size_t i = 0; i < rm->rm_stack_size; ++i) {
	   fprintf(stream, "    %"PRIu64"\n", rm->stack[i]);
	}
    } else {	
	fprintf(stream, "[empty]\n");
    }   
}

// * Load the program from rm bytecode into rm->program
void rm_load_program_from_file(Rm *rm, const char* filepath) {
    FILE *f = fopen(filepath, "rb");
    if(f == NULL) {
	fprintf(stderr, "ERROR: could not open file `%s`: %s\n", filepath, strerror(errno));
	exit(1);
    }

    // * Read the meta Information about file
    Rm_File_Meta meta = {0};
    size_t n = fread(&meta, sizeof(meta), 1, f);
    if(n < 1) {
	fprintf(stderr, "ERROR: could not read file meta `%s`: %s\n", filepath, strerror(errno));
	exit(1);	
    }

    if(meta.magic != RM_FILE_MAGIC) {
	fprintf(stderr,
	"ERROR: %s does not appear to be valid BM file"
	"Unexpected magic %04X. Expected %04X\n",
	filepath, meta.magic, RM_FILE_MAGIC);
	exit(1);	
    }

    // printf("program size: %ld\n", meta.program_size);
    rm->rm_program_size = fread(rm->program, sizeof(rm->program[0]), meta.program_size, f);
    if(meta.program_size != rm->rm_program_size) {
	fprintf(stderr,
	        "ERROR: %s read %"PRIu64" instructions, Expected %"PRIu64" instructions",
	        filepath, rm->rm_program_size, meta.program_size);
	exit(1);
    }
    
}

Err rm_execute_program(Rm *rm, int limit) {
    while(limit != 0 && !rm->halt) {
	Err err = rm_execute_inst(rm);
	if(err != ERR_OK) {
	    printf("ERROR: %s\n", err_as_cstr(err));
	    return err;
	}
	if(limit > 0) {
	    --limit;
	}
    }
    return ERR_OK;
}

Err rm_execute_inst(Rm *rm) {
    if(rm->ip > rm->rm_program_size) {
	return ERR_ILLEGAL_INST;
    }
    
    Inst inst = rm->program[rm->ip];
    // printf("Cur Inst: %s\n", inst_as_cstr(inst.inst_type));
    switch(inst.inst_type) {
    case INST_NOP: {
	rm->ip += 1;
    } break;

    case INST_HALT: {	  
	rm->halt = true;	    
	rm->ip += 1;	    
    } break;	

    case INST_PUSH: {
	if(rm->rm_stack_size >= RM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}
	rm->stack[rm->rm_stack_size++] = (int64_t)inst.inst_operand;
	rm->ip += 1;
    } break;

    case INST_DUP: {
	if(rm->rm_stack_size >= RM_STACK_CAPACITY) {
	    return ERR_STACK_OVERFLOW;
	}

	uint64_t pos = inst.inst_operand;
	if(pos >= rm->rm_stack_size) {
	    return ERR_STACK_UNDERFLOW;
	}
	const uint64_t idx = rm->rm_stack_size - 1 - inst.inst_operand;
	rm->stack[rm->rm_stack_size++] = rm->stack[idx];
	rm->ip += 1;
    } break;

    case INST_JMP: {
	rm->ip = inst.inst_operand;	    
    } break;	

    case INST_PLUSI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op + second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;

    case INST_MINUSI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op - second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;


    case INST_MULI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op * second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;


    case INST_DIVI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op / second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;

    case INST_MODI: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op % second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;
    } break;	

    default:
    	printf("Unknown Instruction\n");

    }
    return ERR_OK;
}

// * Creates a bytecode executables
void rasm_save_to_file(Rm *rm, String_View filepath) {
    const char *filepath_cstr = arena_sv_to_cstr(rm, filepath);

    FILE *file_fd = fopen(filepath_cstr, "wb");
    if(file_fd == NULL) {
	fprintf(stderr, "ERROR: Could Not write to File %s\n", strerror(errno));
	exit(1);
    }

    // * save program metadata
    Rm_File_Meta meta = {
	.magic = RM_FILE_MAGIC,
	.program_size = rm->rm_program_size
    };
    fwrite(&meta, sizeof(meta), 1, file_fd);
    if(ferror(file_fd)) {
	fprintf(stderr, "ERROR: Could Not write to File %s\n", strerror(errno));
	exit(1);	
    }
    
    // * Write the program to file
    fwrite(rm->program, sizeof(rm->program[0]), rm->rm_program_size, file_fd);
    if(ferror(file_fd)) {
	fprintf(stderr, "ERROR: Could Not write to File %s\n", strerror(errno));
	exit(1);
    }
    
    fclose(file_fd);
}

#endif // RM_IMPLEMENTATION
