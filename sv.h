#ifndef SV_H_
#define SV_H_

#include<string.h>

typedef struct {
    size_t count;
    const char* data;
} String_View;

String_View SV(const char* str);
String_View sv_chop_by_delim(String_View *sv, char delim);

#endif // SV_H_

#ifdef SV_IMPLEMENTATION

String_View SV(const char* str) {
    return (String_View) {
	.count = strlen(str),
	.data = str
    };
}

String_View sv_chop_by_delim(String_View *sv, char delim) {
    size_t i = 0;
    while(i < sv->count && sv->data[i] != delim) {
	i += 1;
    }

    // printf("I: %d\n", i);

    String_View res = {
	.data = sv->data,
	.count = i,
    };
    
    if(i < sv->count) {
	sv->data += i + 1;
	sv->count -= i + 1;
    }
    else {
	// * At the end
	sv->data += i;
	sv->count -= i;
    }

    return res;
}

#endif // SV_IMPLEMENTATION
