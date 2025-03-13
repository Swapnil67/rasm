#ifndef SV_H_
#define SV_H_

#include<string.h>
#include<ctype.h>
#include<stdbool.h>

typedef struct {
    size_t count;
    const char* data;
} String_View;

String_View SV(const char* str);
String_View sv_chop_by_delim(String_View *sv, char delim);

String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
bool sv_eq(String_View a, String_View b);

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

String_View sv_trim_left(String_View sv) {
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[i])) {
	i++;
    }

    sv.data += i;
    sv.count -= i;
    return sv;
}


String_View sv_trim_right(String_View sv) {
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[sv.count - i - 1])) {
	i++;
    }

    sv.count -= i;
    return sv;
}

String_View sv_trim(String_View sv) {
    return sv_trim_right(sv_trim_left(sv));
}

bool sv_eq(String_View a, String_View b) {
    if(a.count != b.count) return false;
    return (memcmp(a.data, b.data, a.count) == 0);
}


#endif // SV_IMPLEMENTATION
