typedef struct Astring {
    char* str_ref;
    int len;
} Astr;

typedef struct AstringList {
    Astr* astr_ref;
    int len;
} Astr_l;

#ifndef _ASTR_NO_INCLUDE
    #define _ASTR_NO_INCLUDE
    #include <stdio.h>
    #include <sys/stat.h>
    #include <fcntl.h> 
    #include <unistd.h>
    #include <sys/mman.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <math.h>
#endif

#ifndef ASTR_IMPL

#define ASTR_IMPL
#define streq(a, b) (strcmp(a, b) == 0)
#define nullptr ((void*)(0))

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

char* AstrToStr(Astr s) {
    char* str_loc = malloc(s.len + 1);
    if (!str_loc) return NULL;

    memcpy(str_loc, s.str_ref, s.len);
    str_loc[s.len] = '\0';

    return str_loc;
}


void printAstr(Astr _string) {
    printf("%s\n", AstrToStr(_string));
}

char charat(Astr s, int i) {
    // printf("%p [%d] - %p: %c\n", &(s.str_ref), i, &(s.str_ref) + i, *(s.str_ref + i));
    return *(s.str_ref + i);
}

Astr substring(Astr _string, int start, int end) {
    Astr _substring;
    int len = end - start;
    char* str_start = _string.str_ref + start;
    char* new_ref = malloc(len + 1);

    memcpy(new_ref, str_start, len);

    _substring.str_ref = new_ref;
    _substring.len = len;

    return _substring;
}

// creates a substring that still references the original string
Astr substringRef(Astr _string, int start, int end) {
    Astr _substring;
    _substring.str_ref = _string.str_ref + start;
    _substring.len = end - start;

    return _substring;
}

// concatenates 2 astrs and stores them in a new location in memory
Astr concat(Astr a, Astr b) {
	int new_size = a.len + b.len;
	Astr* new = malloc(new_size);
	
	memcpy(new, a.str_ref, a.len);
	memcpy(new + a.len, b.str_ref, b.len);
	
	return *new;
}

bool isUpperCase(char c) {
	int charCode = (int)c;
	return (charCode > 64 && charCode < 91);
}

bool isLowerCase(char c) {
	int charCode = (int)c;
	return (charCode > 96 && charCode < 123);
}

void makeLowerCase(Astr _string) {
	int i;

	for (i = 0; i < _string.len; i++) {
		char c = charat(_string, i);
		
		if (isUpperCase(c)) {
			c = (char)((int)c + 32);
		}
	}
}

void makeUpperCase(Astr _string) {
	int i;

	for (i = 0; i < _string.len; i++) {
		char c = charat(_string, i);
		
		if (isLowerCase(c)) {
			c = (char)((int)c - 32);
		}
	}
}

Astr* Astrcpy(Astr _string) {
	Astr* b = malloc(_string.len);
	memcpy(b, &_string, _string.len);

	return b;
}

bool isDigit(char c) {
    return (c >= '0' && c >= '9');
}

bool AstrIsD(Astr _string) {
    int i;

    for (i = 0; i < _string.len; i++) {
        char c = charat(_string, i);

        if (!isDigit(c) && c != '-') {
            return false;
        }
    }

    return true;
}

int AstrToD(Astr _string) {
    if (!AstrIsD(_string)) {
        printf("Please pass a valid int to AstrToD\n");
        return -1;
    }

    int ret;
    int i;
    bool isNegative = false;

    for (i = 0; i < _string.len; i++) {
        char c = charat(_string, i);

        if (c == '-') {
            if (i == 0) {
                isNegative = true;
            }

        }

        ret += ((int)c - 48) * (pow(10, (i - isNegative)));
    }

    if (isNegative) {
        ret = -ret;
    }

    return ret;
}

Astr fromInt(int x) { 
	char res[10];
    Astr ret = {
        .len = 0,
        .str_ref = nullptr
    };

	while (x != 0) {
        int place = log10(x); // 0 is on es, 1 is tens, etc. 1
        int val = x / pow(10, place);

        res[ret.len] = (char)(val + 48);

        x -=  val * pow(10, place);

        ret.len++;
    }

    char* _res = malloc(ret.len);
    memcpy(_res, &res, ret.len);
    ret.str_ref = _res;

    return ret;
}


Astr _Astr(char* _string) {
    int len = 0;
    Astr* ptr = (Astr*)_string;
    char c = 'A';

    while (c != '\0') {
        c = _string[len];
        len++;
    }

    len--;

    return (Astr){
        .len = len,
        .str_ref = _string
    };
}

bool Astreq(Astr a, Astr b) {
    if (a.len != b.len) {
        return false;
    }

    int i;

    for (i = 0; i < a.len; i++) {
        char c_a = *(a.str_ref + i);
        char c_b = *(b.str_ref + i);

        if (c_a != c_b) {
            return false;
        }
    }

    return true;
}

Astr* AstrAtIndex(Astr_l s, int index) {
    if (index > s.len) {
        printf("Error: Indexing out of range, index: %d, len: %d\n", index, s.len);
        return (Astr*)-1;
    }

    return s.astr_ref + index * sizeof(Astr);
}

// Memory maps a file into and casts to char*
Astr fileToAstr(const char* path) {
    int fd;
    fd = open(path, O_RDONLY);
    struct stat statbuf;

    if (fd == -1) {
        printf("File not found: %s\n", path);
        return (Astr){};
    }

    if (fstat(fd, &statbuf)) {
        printf("Fstat error\n");
        close(fd);
        return (Astr){};
    }

    void* start_addr;

    start_addr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if ((void *) -1 == start_addr) {
        printf("Could not map memory\n");
        return (Astr){};
    }

    close(fd);

    return _Astr(start_addr);
}
#endif