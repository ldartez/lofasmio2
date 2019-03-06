// main.cpp for readlofasm tool

#include <iostream>
#include <cstdio>
#include <fstream>
#include "lofasmio/lofasmfileheader.h"
#include "lofasmio/integration.h"
#include "lofasmio/parse_raw.h"
#include <zlib.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <time.h>
#include <math.h>

using namespace std;

double get_time(){
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec + (tp.tv_nsec / pow(10,9));
}

int main(int argc, char** argv){
    if (argc < 2){
        cout << "Usage: " << argv[0] << " filename";
        cout << endl;
        return 0;
    }
    gzFile infile = gzopen(argv[1], "rb");
    if (infile == nullptr){
        cerr << "Unable to open file." << endl;
        return -1;
    }
    char* buffer = (char *) malloc(lofasm::BURST_SIZE);
    gzread(infile, buffer, 128); // read header string
    cout << "Reading " << argv[1] << endl;


    lofasm::Lofasm_FHDR hdr(buffer);
    hdr.print();
    int n = 0;
    int nerr = 0;

    double dataread_start = get_time();
    gzread(infile, buffer,lofasm::BURST_SIZE);
    vector<vector<double>>* v;
    while(!gzeof(infile)){
        lofasm::LofasmSubintBurst burst(buffer);
        if (!burst.isValid()){
            nerr++;
            printf("Burst interrupted at %d\n", burst.getInterrupt());
            // reset file location
            gzseek(infile, -1*(gztell(infile)-burst.getInterrupt()), SEEK_CUR);
        }
        v = new vector<vector<double>>(10);
        burst.parse(*v);
        delete v;
        gzread(infile, buffer,lofasm::BURST_SIZE);
    }
    double dataread_end = get_time();
    printf("Number of interrupts: %d\n", nerr);
    printf("Read and parsed data in %f s.\n", dataread_end-dataread_start);


    free(buffer);
    return 0;
}

