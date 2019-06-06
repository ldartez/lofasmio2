#include "lofasmio/lofasmreader.h"
#include "lofasmio/integration.h"
#include "lofasmio/time.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <zlib.h>
#include <iostream>

using namespace lofasm;

// class definition for lofasm data streamer
LofasmStream::LofasmStream(const char* fname){
    this->fname = fname;
    file = gzopen(fname, "rb"); // open data file
    // file validation; make sure we can open the file
    if (file == nullptr){
        std::cerr << "Unable to open file " << file << "." << std::endl;
        return;
    }
    /*
     read first 24 bytes of file
     the file header preamble consists of 3 fields of 8characters.
     field 1: file signature
     field 2: header version
     field 3: total header length

     the total header length represents the total number of bytes in the header.
     this total includes the preamble. 
    */
    gzread(file, fSig, FHDR_ENTRY_LENGTH_BYTES); // load file sig
    gzread(file, fHdrVer, FHDR_ENTRY_LENGTH_BYTES); // load file header version
    gzread(file, fHdrLen, FHDR_ENTRY_LENGTH_BYTES); // load file header version

    // check file signature
    if (strncmp(fSig, "    LoCo", FHDR_ENTRY_LENGTH_BYTES)){
        std::cout << "File signature does not match lofasm filetype: ";
        std::cout << fSig << "." << std::endl;
    }
    // check file header version
    if (strncmp(fHdrVer, "       5", FHDR_ENTRY_LENGTH_BYTES)){
        std::cout << "File header version is not 5!: ";
        std::cout << fHdrVer << "." << std::endl;
    }
    // check file header length
    int hdrLen = atoi(fHdrLen);
    if (hdrLen != FHDRV5_LENGTH_BYTES){
        std::cout << "File header length is not ";
        std::cout << FHDRV5_LENGTH_BYTES<< "!: ";
        std::cout << fHdrLen << "." << std::endl;
    }
    // copy inital header fields into file header buffer
    hdr_buf = (char *) malloc(hdrLen); // allocate space for file header string
    memcpy(hdr_buf, fSig, FHDR_ENTRY_LENGTH_BYTES);
    memcpy(hdr_buf+FHDR_ENTRY_LENGTH_BYTES, fHdrVer, FHDR_ENTRY_LENGTH_BYTES);
    memcpy(hdr_buf+2*FHDR_ENTRY_LENGTH_BYTES, fHdrLen, FHDR_ENTRY_LENGTH_BYTES);

    int rBytes = hdrLen - 3*FHDR_ENTRY_LENGTH_BYTES; // remaining bytes in file header
    gzread(file, hdr_buf+3*FHDR_ENTRY_LENGTH_BYTES, rBytes);
    Lofasm_FHDR hdr(hdr_buf);
    //hdr.print();
    burst_buf = (char *) malloc(BURST_SIZE);
}

LofasmStream::~LofasmStream(){
    std::cout << "Destroying lofasm stream object." << std::endl;
    free(hdr_buf);
    free(burst_buf);
}

std::vector<std::vector<double>> LofasmStream::read(size_t N){
    /* read specified number of LoFASM integrations
    */

    gzread(file, burst_buf, BURST_SIZE);
    LofasmSubintBurst burst(burst_buf);
    burst.parse(*poldata);
    return *poldata;
}
//double** LofasmStream::read(){}