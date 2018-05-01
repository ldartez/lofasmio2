/*
  parse_raw.cpp

  Routines to for converting binary values into
  native data types.
*/
#include "lofasmio/parse_raw.h"

/*
  convert big endian binary representation to
  little endian integer value
*/
unsigned int binary_to_uint(const char* raw, int n){
    unsigned int result = 0;
    char* p = (char*) (&result);
    for (int i=0; i<n; ++i){
        p[i] = raw[n-1-i];
    }
    return result;
}

int binary_to_int(const char* raw, int n){
    int result = 0;
    char* p = (char*) (&result);
    for (int i=0; i<n; ++i){
        p[i] = raw[n-1-i];
    }
    return result;
}
