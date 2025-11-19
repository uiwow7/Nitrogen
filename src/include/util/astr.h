typedef struct Astring {
    char *str_ref;
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

/// @brief Checks if a given file exists (Stolen from StackOverflow)
/// @param filename The path to the file
/// @return Whether or not the file specified in `filename` exists
bool file_exists(char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

/// @brief Converts an Astr struct into a normal c-style string
/// @param _string The Astr to convert
/// @return The converted string
char *AstrToStr(Astr _string) {
    char *str_loc = malloc(_string.len + 1);
    if (!str_loc) return NULL;

    memcpy(str_loc, _string.str_ref, _string.len);
    str_loc[_string.len] = '\0';

    return str_loc;
}


/// @brief Prints an Astr using printf
/// @param _string The Astr to print
void printAstr(Astr _string) {
    printf("%s\n", AstrToStr(_string));
}

/// @brief Gets a char at an index in an Astr
/// @param _string The string to get the char from
/// @param index The index of the char
/// @return The char at index `index`
char charat(Astr _string, int index) {
    return *(_string.str_ref + index);
}

/// @brief Returns a string that's a part of `_string`, spanning from `start` (inclusive) to `end` (exclusive)
/// @param _string The larger string
/// @param start Start index for the substring (inclusive)
/// @param end End index for the substring (exclusive)
/// @return The substring
Astr substring(Astr _string, int start, int end) {
    Astr _substring;
    int len = end - start;
    char *str_start = _string.str_ref + start;
    char *new_ref = malloc(len + 1);

    memcpy(new_ref, str_start, len);

    _substring.str_ref = new_ref;
    _substring.len = len;

    return _substring;
}

// creates a substring that still references the original string


/// @brief Returns a string that's a part of `_string` that references the original string data from `_string`, spanning from `start` (inclusive) to `end` (exclusive)
/// @param _string The larger string
/// @param start Start index for the substring (inclusive)
/// @param end End index for the substring (exclusive)
/// @return The substring
Astr substringRef(Astr _string, int start, int end) {
    Astr _substring;
    _substring.str_ref = _string.str_ref + start;
    _substring.len = end - start;

    return _substring;
}

/// @brief Concatenates 2 Astrs and stores them into a new location in memory, then returns that location dereferenced
/// @param a A string to concatenate
/// @param b A string to concatenate
/// @return A + B concatenated
Astr concat(Astr a, Astr b) {
	int new_size = a.len + b.len;
	char *new_ref = malloc(new_size);
	
	memcpy(new_ref, a.str_ref, a.len);
	memcpy(new_ref + a.len, b.str_ref, b.len);
	
	Astr new = {
        .str_ref = new_ref,
        .len = new_size
    };

    return new;
}

/// @brief WARNING: Only use this if `a` and `b` are `free`-able
///
/// Concatenates 2 Astrs and stores them into a new location in memory, then returns that location dereferenced, but frees the original strings.
/// @param a MUST BE `free`-able. A string to concatenate
/// @param b MUST BE `free`-able. A string to concatenate
/// @return A + B concatenated
Astr concatFree(Astr a, Astr b) {
	int new_size = a.len + b.len;
	Astr *new = malloc(new_size);
	
	memcpy(new, a.str_ref, a.len);
	memcpy(new + a.len, b.str_ref, b.len);

    free(a.str_ref);
    // free(b.str_ref);
	
	return *new;
}

/// @brief Concatenates 2 Astrs and stores the result in `a`
/// @param a The 1st string to concat and where the result will be stored
/// @param b The 2nd string to concat
void concatAppend(Astr a, Astr b) {
    char *a_ref = realloc(a.str_ref, a.len + b.len + 1);

    memcpy(a_ref, a.str_ref, a.len);
	memcpy(a_ref + a.len, b.str_ref, b.len);

    a.str_ref = a_ref;
}

/// @brief Returns if a char `c` is upper case or not
/// @param c The char to check
/// @return Is `c` uppercase?
bool isUpperCase(char c) {
	int charCode = (int)c;
	return (charCode > 64 && charCode < 91);
}

/// @brief Returns if a char `c` is lower case or not
/// @param c The char to check
/// @return Is `c` lowercase?
bool isLowerCase(char c) {
	int charCode = (int)c;
	return (charCode > 96 && charCode < 123);
}

/// @brief Makes an Astr into lower case. This modifies the original string data.
/// @param _string The string to make lower case
void makeLowerCase(Astr _string) {
	int i;

	for (i = 0; i < _string.len; i++) {
		char c = charat(_string, i);
		
		if (isUpperCase(c)) {
			c = (char)((int)c + 32);
		}
	}
}

/// @brief Makes an Astr into upper case. This modifies the original string data.
/// @param _string The string to make upper case
void makeUpperCase(Astr _string) {
	int i;

	for (i = 0; i < _string.len; i++) {
		char c = charat(_string, i);
		
		if (isLowerCase(c)) {
			c = (char)((int)c - 32);
		}
	}
}

/// @brief Copies an Astr and returns a pointer to the copy
/// @param _string The Astr to copy
/// @return A pointer to the newly created copy
Astr* Astrcpy(Astr _string) {
	Astr* b = malloc(_string.len);
	memcpy(b, &_string, _string.len);

	return b;
}

/// @brief Returns if `c` is a digit
bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}

/// @brief Returns whether or not an Astr `_string` is an integer
/// @param _string The Astr to check
/// @return Is `_string` an integer?
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

/// @brief Converts an Astr into an integer
/// @param _string The Astr to convert
/// @return The converted integer. Returns INT_MIN in case of failiure.
int AstrToD(Astr _string) {
    if (!AstrIsD(_string)) {
        printf("Please pass a valid int to AstrToD\n");
        return _SC_INT_MIN;
    }

    int ret = 0;
    int i;
    bool isNegative = false;

    for (i = 0; i < _string.len; i++) {
        char c = charat(_string, i);

        if (c == '-') {
            if (i == 0) {
                isNegative = true;
            }

        }

        int charnum = (int)c - 48;
        int place = _string.len - 1 - i - isNegative;
        int placeFactor = pow(10, place);

        ret += (charnum * placeFactor);
    }

    if (isNegative) {
        ret = -ret;
    }
    return ret;
}

/// @brief Generates an Astr from a number
/// @param x The number to convert into an Astr
/// @return `x` as an Astr
Astr fromInt(int x) { 
	char res[10];
    Astr ret = {
        .len = 0,
        .str_ref = NULL
    };

    int prev_x = x;

	while (x != 0) {
        int place = log10(x); // 0 is ones, 1 is tens, etc. 1
        int val = x / pow(10, place);

        res[ret.len] = (char)(val + 48);

        x -=  val * pow(10, place);
        
        int num_zeroes;
        if (x == 0) {
            num_zeroes = log10(prev_x);
        } else {
            num_zeroes = log10(prev_x) - log10(x); 
        }

        for (int i = 0; i < num_zeroes; i++) {
            ret.len++;
            res[ret.len] = '0';
        }

        ret.len++;
    }

    char *_res = malloc(ret.len);
    memcpy(_res, &res, ret.len);
    ret.str_ref = _res;

    return ret;
}


/// @brief Converts a regular c-style (null-terminated) string into an Astr
/// @param _string The c-style string to convert
/// @return The converted Astr
Astr _Astr(char *_string) {
    return (Astr){
        .len = strlen(_string),
        .str_ref = _string
    };
}

/// @brief Checks if 2 Astrs' data is equal
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

/// @brief Returns an Astr at a given index in a list of Astrs
/// @param _string_list The Astr_l to pull from
/// @param index The index of the Astr
/// @return A pointer to the Astr at `index` in `_string_list`, or (Astr*)-1 in case of failiure
Astr *AstrAtIndex(Astr_l _string_list, int index) {
    if (index > _string_list.len) {
        printf("Error: Indexing out of range, index: %d, len: %d\n", index, _string_list.len);
        return (Astr*)-1;
    }

    return _string_list.astr_ref + index;
}

/// @brief Reads a file from a path and converts it into an Astr
/// @param path The file to read from
/// @return An Astr of the file contents or (Astr){} in case of failiure
Astr fileToAstr(const char *path) {
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

/// @brief Returns if a char is a tab, newline, or space
bool isWhiteSpace(char c) {
    return (
        c == ' '  ||
        c == '\t' ||
        c == '\n'
    );
}

#endif