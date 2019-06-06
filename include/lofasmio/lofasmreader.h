// lofasmreader.h

#ifndef LOFASMREADER_H
#include "lofasmfileheader.h"
#include "integration.h"
#include "time.h"
#include <zlib.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace lofasm{
    class LofasmStream{
        private:
        std::string fname; // holder for filename
        char fSig[FHDR_ENTRY_LENGTH_BYTES+1]; // file signature placeholder
        char fHdrVer[FHDR_ENTRY_LENGTH_BYTES+1]; // file header version placeholder
        char fHdrLen[FHDR_ENTRY_LENGTH_BYTES+1]; // file header length placeholder
        char* hdr_buf; // buffer for file header string
        char* burst_buf; // buffer for raw burst string
        gzFile file; // file handle for data
        LofasmSubintBurst burst;
        std::vector<std::vector<double>>* poldata;

        public:
        LofasmStream(const char*); // constructor
        ~LofasmStream(); // destructor
        //double** read(); // read entire file data
        std::vector<std::vector<double>> read(size_t); // read specified number of spectra
        //void close(); // close file handles
    };
}

#define LOFASMREADER_H
#endif