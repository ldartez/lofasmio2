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
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

// prototypes
void writeHeaderBBX(gzFile, lofasm::Lofasm_FHDR&, int);
double get_time();

int main(int argc, char** argv){
    if (argc < 2){
        cout << "Usage: " << argv[0] << " filename";
        cout << endl;
        return 0;
    }
    // create local bbx directory if it doesn't already exist.
    struct stat statbuf;
    int statcode, burst_interrupt;
    string rootDir = "bbx/"; // relative to 'here'
    statcode = stat(rootDir.c_str(), &statbuf);

    // if root directory does not exist then create it and the polarization
    if (statcode == -1)
        mkdir(rootDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    // check if polarization directories exist and create them if they don't
    string dirName;
    for (int i=0; i<lofasm::POLS.size(); ++i){
        dirName = lofasm::POLS[i];
        statcode = stat((rootDir+dirName).c_str(), &statbuf);
        if (statcode == -1)
            mkdir((rootDir+dirName).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
    }

    // iterate over input files and parse them
    for (int k=1; k<argc; ++k){
        string inputfilename = argv[k];
        gzFile infile = gzopen(inputfilename.c_str(), "rb");
        if (infile == nullptr){
            cerr << "Unable to open file " << inputfilename <<  endl;
            continue;
        }
        char* buffer = (char *) malloc(lofasm::BURST_SIZE);
        gzread(infile, buffer, 128); // read header string
        cout << "Reading " << inputfilename << endl;

        lofasm::Lofasm_FHDR hdr(buffer);
        hdr.print();
        int n=0;
        int nerr = 0;

        double dataread_start = get_time();
        // read first burst into buffer
        gzread(infile, buffer,lofasm::BURST_SIZE);
        vector<vector<double>>* pv;
        double* pdata;
        vector<vector<double>> v;

        /*
          Open output files.
        */
        vector<gzFile> outFiles;
        string ofname;
        for(int i=0; i<lofasm::POLS.size(); ++i){
            ofname = rootDir + lofasm::POLS[i] + "/";
            ofname += to_string(hdr.mjd) + "_" + lofasm::POLS[i] + ".bbx.gz";
            gzFile of = gzopen(ofname.c_str(), "wb");
            writeHeaderBBX(of, hdr, i);
            outFiles.push_back(of);
        }

        cout << "Burst Key: " << lofasm::BURST_KEY << endl;
        pv = new vector<vector<double>>(10);
        while(!gzeof(infile)){
            //for (int w=0; w<20; ++w){
            //cout << "iteration " << w << endl;
            lofasm::LofasmSubintBurst burst(buffer);
            if (!burst.isValid()){
                nerr++;
                if (burst.getInterrupt() != -1){
                    // move file location to start of valid burst header packet
                    // and continue reading data at the new position
                    int offset = (lofasm::PACKETS_PER_BURST - (burst.getInterrupt()/lofasm::PACKET_SIZE))*lofasm::PACKET_SIZE;
                    cout << "Interrupted Burst Buffer: correcting file location from ";
                    cout << gztell(infile) << " to " << gztell(infile)-offset << endl;
                    gzseek(infile, -1*offset, SEEK_CUR);
                    gzread(infile, buffer, lofasm::BURST_SIZE);
                    continue;
                }
                else{
                    /*
                      interrupt = -1.
                      no burst key was detected.
                      file may be corrupted or pointer placement is wrong.
                      skip the rest of this file.
                    */
                    cout << "File is corrupt " << inputfilename << " ...skipping\n";
                    break; // out of while loop to skip the rest of the current file
                }
            }
            /*
              burst is valid
            */
            //cout << "Valid burst at " << gztell(infile)-lofasm::BURST_SIZE << endl;
            v = *pv;
            burst.parse(v);

            // write pols
            for (int i=0; i<v.size(); ++i){
                pdata = v[i].data(); // get pointer to data array
                gzwrite(outFiles[i], (char*) pdata, sizeof(double)*v[i].size());
            }
            gzread(infile, buffer, lofasm::BURST_SIZE);
        }
        delete pv;

        double dataread_end = get_time();
        printf("Number of interrupts: %d\n", nerr);
        printf("Read and parsed data in %f s.\n", dataread_end-dataread_start);
        // free buffers & close files
        free(buffer);
        for (int i=0; i<outFiles.size(); ++i) gzclose(outFiles[i]);
        gzclose(infile);
    }

    // print debug info

    return 0;
}

void writeHeaderBBX(gzFile of, lofasm::Lofasm_FHDR& hdr, int chanID){
    //
    //  write BBX header to file.
    //
    string x, chanLabel;
    const char* pol = lofasm::POLS[chanID];
    //x = (string) "%" + to_string(0x02) + (string) "BX\n";
    x = "%\002BX\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%hdr_type: LoFASM-filterbank\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%hdr_version: 1\n"; // THIS SHOULD BE CONFIGURABLE!
    gzwrite(of, x.c_str(), x.length());
    x = "%station: " + to_string(hdr.station_id) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%channel: " + (string) pol + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%channel_label: " + hdr.get_channel_label(pol) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%start_mjd: " + to_string(hdr.mjd) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%time_offset_J2000: 0 (s)\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%frequency_offset_DC: 0 (Hz)\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%dim1_label: time (s)\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%dim1_start: " + to_string(hdr.mjd - (2451545 - 2400000)) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%dim1_span: " + to_string(hdr.int_time*hdr.num_samples) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%dim2_label: frequency (Hz)\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%dim2_start: " + to_string(hdr.fstart) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%dim2_span: " + to_string(hdr.fstep*hdr.num_freq) + "\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%data_label: ";
    if (pol[0] == pol[1]) x += "power spectrum (arbitrary)\n";
    else x += "cross spectrum (arbitrary)\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%data_offset: 0\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%data_scale: 1\n";
    gzwrite(of, x.c_str(), x.length());
    x = "%data_type: real64\n";
    gzwrite(of, x.c_str(), x.length());
    x = to_string(hdr.num_samples) + " " + to_string(hdr.num_freq) + " ";
    gzwrite(of, x.c_str(), x.length());
    if (pol[0] == pol[1]) x = "1 ";
    else x = "2 ";
    x += "64 raw256\n";
    gzwrite(of, x.c_str(), x.length());

    return;
}

double get_time(){
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec + (tp.tv_nsec / pow(10,9));
}

