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
#define RM_BINDING_CAPACITY 1024
#define RM_DEFERRED_OPERAND_CAPACITY 1024
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
    INST_JMPIF,

    INST_PLUSI,
    INST_MINUSI,
    INST_MULI,
    INST_DIVI,
    INST_MODI,

    INST_GT,
    INST_GTE,
    INST_LT,
    INST_LTE,
} Inst_Type;

typedef uint64_t Inst_Addr;

typedef struct {
    Inst_Type  inst_type;
    uint64_t   inst_operand;
} Inst;

typedef struct {
    Inst_Addr addr;
    String_View name;
} Deferred_Operand;

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
    String_View name;
    Inst_Addr value;
} Binding;

typedef struct {
    int64_t stack[RM_STACK_CAPACITY];    
    uint64_t rm_stack_size;
    
    Inst program[RM_PROGRAM_CAPACITY];
    uint64_t rm_program_size;
    uint64_t ip;

    Binding bindings[RM_BINDING_CAPACITY];
    size_t bindings_size;
    
    Deferred_Operand deferred_operands[RM_DEFERRED_OPERAND_CAPACITY];
    size_t deferred_operands_size;

    char arena[RM_ARENA_CAPACITY];
    size_t arena_size;

    bool halt;
} Rm;

void *arena_sv_to_cstr(Rm *rm, String_View sv);
void *arena_alloc(Rm *rm, size_t n);
String_View arena_slurp_file(Rm *rm, String_View filepath);


bool resolve_bind_value(Rm *rm, String_View name, Inst_Addr *addr);
bool rasm_bind_value(Rm *rm, String_View name);
void rasm_push_deferred_operand(Rm *rm, String_View operand, Inst_Addr addr);

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
    case ERR_OK:		return "ERR_OK";
    case ERR_ILLEGAL_INST:	return "ERR_ILLEGAL_INST";
    case ERR_STACK_OVERFLOW:	return "ERR_STACK_OVERFLOW";
    case ERR_STACK_UNDERFLOW:	return "ERR_STACK_UNDERFLOW";
    default:
	return "Unknown Err";
    }
}

const char* inst_to_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP:	return "INST_NOP";
    case INST_HALT:	return "INST_HALT";
    case INST_PUSH:	return "INST_PUSH";
    case INST_DUP:	return "INST_DUP";
    case INST_JMP:	return "INST_JMP";
    case INST_JMPIF:	return "INST_JMPIF";
    
    case INST_PLUSI:	return "INST_PLUSI";
    case INST_MINUSI:	return "INST_MINUSI";
    case INST_MULI:	return "INST_MULI";
    case INST_DIVI:	return "INST_DIVI";
    case INST_MODI:	return "INST_MODI";
    
    case INST_GT:	return "INST_GT";
    case INST_GTE:	return "INST_GTE";
    case INST_LT:	return "INST_LT";
    case INST_LTE:	return "INST_LTE";
default:
    return "Unknown type";
    }
}

const char* inst_as_cstr(Inst_Type type) {
    switch(type) {
    case INST_NOP:	return "nop";
    case INST_HALT:	return "halt";
    case INST_PUSH:	return "push";
    case INST_DUP:	return "dup";
    case INST_JMP:	return "jmp";
    case INST_JMPIF:	return "jmp_if";
    
    case INST_PLUSI:	return "plusi";
    case INST_MINUSI:	return "minusi";
    case INST_MULI:	return "muli";
    case INST_DIVI:	return "divi";
    case INST_MODI:	return "modi";
    
    case INST_GT:	return "gt";
    case INST_GTE:	return "gte";
    case INST_LT:	return "lt";
    case INST_LTE:	return "lte";
default:
    return "Unknown type";
    }
}

bool inst_has_operand(Inst_Type type) {
    switch(type) {
    case INST_NOP:	return false;
    case INST_HALT:	return false;
    case INST_PUSH:	return true;
    case INST_DUP:	return true;
    case INST_JMP:	return true;
    case INST_JMPIF:	return true;
    
    case INST_PLUSI:	return false;
    case INST_MINUSI:	return false;
    case INST_MULI:	return false;
    case INST_DIVI:	return false;
    case INST_MODI:	return false;
    
    case INST_GT:	return false;
    case INST_GTE:	return false;
    case INST_LT:	return false;
    case INST_LTE:	return false;
    
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

// static void show_bindings(Rm *rm) {
//     printf("\n ------ Bindings ----- \n");
//     for(size_t i = 0; i < rm->bindings_size; ++i) {
// 	printf("Name: "SV_Fmt", addr: %"PRIu64"\n",
// 	        SV_Arg(rm->bindings[i].name), rm->bindings[i].value);
//     }    
// }


// static void show_deferred_operands(Rm *rm) {
//     printf("\n ------ Deferred_Operands ----- \n");
//     for(size_t i = 0; i < rm->deferred_operands_size; ++i) {
// 	printf("Name: "SV_Fmt", addr: %"PRIu64"\n",
// 	        SV_Arg(rm->deferred_operands[i].name), rm->deferred_operands[i].addr);
//     }
// }


// * Add new deferred_operand to deferred_operands array
void rasm_push_deferred_operand(Rm *rm, String_View operand, Inst_Addr addr) {
    assert(rm->deferred_operands_size < RM_DEFERRED_OPERAND_CAPACITY);
    rm->deferred_operands[rm->deferred_operands_size++] = (Deferred_Operand) {
	.addr = addr,
	.name = operand
    };
}

// * Gets the value of bind value to a label
// * Function => address
// * Other    => Literal
// TODO change addr parameter to WORD type
bool resolve_bind_value(Rm *rm, String_View name, Inst_Addr *addr) {
    for(size_t i = 0; i < rm->bindings_size; ++i) {
	if(sv_eq(name, rm->bindings[i].name)) {
	    *addr = rm->bindings[i].value;
	    return true;
	}
    }
    return false;
}

// * Binds the label name with it's address
bool rasm_bind_value(Rm *rm, String_View name) {
    // * Check if label already bind

    // TODO change this to WORD
    Inst_Addr ignore;
    if(resolve_bind_value(rm, name, &ignore)) {
	return false;
    }
    
    rm->bindings[rm->bindings_size++] = (Binding) {
	.value = (uint64_t) rm->rm_program_size,
	.name = name
    };
    
    return true;
}

// * Translate RM program from Text To Binary (create .rm bytecode executables)
void rasm_translate_source(Rm *rm, String_View input_filepath) {
    // * Load the program from file
    String_View original_source = arena_slurp_file(rm, input_filepath);

    int line_number = 0;

    while(original_source.count > 0) {
	String_View line = sv_trim(sv_chop_by_delim(&original_source, '\n'));
	
	line_number += 1;
	// Check if comment
	if(*line.data == RASM_COMMENT_SYMBOL) {
	    continue;
	}

	// printf("Line: "SV_Fmt"\n", SV_Arg(line));
		
	String_View token = sv_trim(sv_chop_by_delim(&line, ' '));
	// printf("Token: "SV_Fmt"\n", SV_Arg(token));

	if(token.count > 0 && *token.data == RASM_PP_SYMBOL) {
	    // TODO Check for pre-processors
	}

	// * Check for labels
	if(token.data[token.count - 1] == ':') {
	    String_View name = {
		.count = token.count - 1,
		.data = token.data
	    };
	    if(!rasm_bind_value(rm, name)) {
		fprintf(stderr, ""SV_Fmt":%d: ERROR: binding `"SV_Fmt"` is already defined\n",
		SV_Arg(input_filepath), line_number, SV_Arg(token));
		exit(1);
	    }

	    // * Check if inst after ':'
	    token = sv_trim(sv_chop_by_delim(&line, ' '));
	}
	
	// Instructions
	if(token.count > 0) {
	    // * Get the operand
	    String_View operand = sv_trim(sv_chop_by_delim(&line, RASM_COMMENT_SYMBOL));
	    // printf("operand: "SV_Fmt"\n", SV_Arg(operand));
	    
	    if(sv_eq(token, SV(inst_as_cstr(INST_PUSH)))) {
		Inst_Type inst_type = INST_PUSH;
		rm->program[rm->rm_program_size].inst_type = inst_type;

		char *str = arena_sv_to_cstr(rm, operand);
		char *endptr;
		int64_t val = strtoll(str, &endptr, 10);
		if(endptr == str) {
		    fprintf(stderr, "No digits were found\n");
		    exit(1);
		}
		rm->program[rm->rm_program_size].inst_operand = (uint64_t) val;
	    }   
	    else if(sv_eq(token, SV(inst_as_cstr(INST_DUP)))) {
		Inst_Type inst_type = INST_DUP;
		rm->program[rm->rm_program_size].inst_type = inst_type;

		char *str = arena_sv_to_cstr(rm, operand);
		char *endptr;
		int64_t val = strtoll(str, &endptr, 10);
		if(endptr == str) {
		    fprintf(stderr, "No digits were found\n");
		    exit(1);
		}
		rm->program[rm->rm_program_size].inst_operand = (uint64_t) val;
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_JMP)))) {
		Inst_Type inst_type = INST_JMP;
		rm->program[rm->rm_program_size].inst_type = inst_type;
		// printf("Token IN: "SV_Fmt"\n", SV_Arg(token));
		
		if(operand.count == 0) {
		    fprintf(stderr,
		            ""SV_Fmt":%d: ERROR: Expected label.\n", SV_Arg(input_filepath), line_number);
		    exit(1);		    
		}

		rasm_push_deferred_operand(rm, operand, rm->rm_program_size);		
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_JMPIF)))) {
		Inst_Type inst_type = INST_JMPIF;
		rm->program[rm->rm_program_size].inst_type = inst_type;
		// printf("Token IN: "SV_Fmt"\n", SV_Arg(token));
		
		if(operand.count == 0) {
		    fprintf(stderr,
       		           ""SV_Fmt":%d: ERROR: Expected label.\n",
		           SV_Arg(input_filepath), line_number);
		    exit(1);		    
		}
		rasm_push_deferred_operand(rm, operand, rm->rm_program_size);		
	    }	    
	    else if(sv_eq(token, SV(inst_as_cstr(INST_PLUSI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_PLUSI;
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_MINUSI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_MINUSI;
	    }
	    else if(sv_eq(token, SV(inst_as_cstr(INST_MULI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_MULI;
	    }	    	       	    
	    else if(sv_eq(token, SV(inst_as_cstr(INST_DIVI)))) {
		rm->program[rm->rm_program_size].inst_type = INST_DIVI;
	    }
    	    else if(sv_eq(token, SV(inst_as_cstr(INST_GTE)))) {
		rm->program[rm->rm_program_size].inst_type = INST_GTE;
	    }	    	       	    
	    else if(sv_eq(token, SV(inst_as_cstr(INST_HALT)))) {
		rm->program[rm->rm_program_size].inst_type = INST_HALT;
	    }
	    else {
		fprintf(stderr, ""SV_Fmt":%d: ERROR unknown instruction `"SV_Fmt"`\n",
		SV_Arg(input_filepath), line_number, SV_Arg(token));
		exit(1);
	    }
	    rm->rm_program_size += 1;
	}
	// printf("------------\n");
    }

    for(size_t i = 0; i < rm->deferred_operands_size; ++i) {
	String_View binding = rm->deferred_operands[i].name;
	Inst_Addr addr = rm->deferred_operands[i].addr;
	if(!resolve_bind_value(rm, binding, &rm->program[addr].inst_operand)) {
	    fprintf(stderr, ""SV_Fmt" ERROR: unknown binding `"SV_Fmt"`\n",
	    SV_Arg(input_filepath), SV_Arg(binding));	    
	    exit(1);	    
	}
    }
    
    // show_bindings(rm);
    // show_deferred_operands(rm);
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

    // * Only for debugging
    // printf("    %s", inst_as_cstr(inst.inst_type));
    // if(inst_has_operand(inst.inst_type)) {
    // 	printf(" %ld", inst.inst_operand);
    // }
    // printf("\n");    

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

    case INST_JMPIF: {
	if(rm->rm_stack_size < 1) {
	    return ERR_STACK_UNDERFLOW;
	}
	rm->rm_stack_size -= 1;		
	// * If the top of the stack is true then jmp
	if((int64_t)rm->stack[rm->rm_stack_size]) {
	    // printf("JMP\n");
	    rm->ip = inst.inst_operand;
	} else {
	    // printf("DON'T JMP\n");	    
	    rm->ip += 1;
	}
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

    case INST_GT: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op > second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;	
    } break;	

    case INST_GTE: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op >= second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;	
    } break;

    case INST_LT: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op < second_op;
	rm->ip += 1;
	rm->rm_stack_size -= 1;	
    } break;	    

    case INST_LTE: {
	if(rm->rm_stack_size < 2) {
	    return ERR_STACK_UNDERFLOW;
	}
	int64_t first_op = (int64_t)rm->stack[rm->rm_stack_size - 2];
	int64_t second_op = (int64_t)rm->stack[rm->rm_stack_size - 1];
	rm->stack[rm->rm_stack_size-2] = first_op <= second_op;
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
