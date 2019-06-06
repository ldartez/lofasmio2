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
#include <iomanip>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
using namespace std;
namespace pt = boost::posix_time;
namespace gd = boost::gregorian;

// global vars
string rootDir = "bbx/"; // relative to 'here'

// prototypes
string createHeaderBBX(lofasm::Lofasm_FHDR&, int);
double get_time();
void dumpBlock(lofasm::Lofasm_FHDR&, vector<ofstream*>&);
pt::ptime mjd2gregorian(double);
string gregorian2string(pt::ptime);

int main(int argc, char** argv){
    if (argc < 2 || argc > 2){
        cout << "Usage: " << argv[0] << " filename";
        cout << endl;
        return 0;
    }
    // create local bbx directory if it doesn't already exist.
    char* c = new char[100];
    string cwd = getcwd(c, 100);
    delete[] c;
    rootDir = cwd+"/"+rootDir;
    cout << "BBX target: " << rootDir << endl;

    struct stat statbuf;
    int statcode, burst_interrupt;
    statcode = stat(rootDir.c_str(), &statbuf);

    // if root directory does not exist then create it
    if (statcode == -1)
        mkdir(rootDir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    // check if polarization directories exist and create them if they don't
    string dirName;
    cout << "Creating bbx directories...\n";
    for (int i=0; i<lofasm::POLS.size(); ++i){
        dirName = lofasm::POLS[i];
        statcode = stat((rootDir+dirName).c_str(), &statbuf);
        if (statcode == -1){
            //cout << "Creating directory: " << rootDir+dirName << endl;
            mkdir((rootDir+dirName).c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
        }
    }

    // iterate over input files
    for (int k=1; k<argc; ++k){
        string inputfilename = argv[k];
        gzFile infile = gzopen(inputfilename.c_str(), "rb");
        if (infile == nullptr){
            cerr << "Unable to open file " << inputfilename <<  endl;
            continue;
        }
        char* buffer = (char *) malloc(lofasm::BURST_SIZE);
        gzread(infile, buffer, 128); // read header string
        cout << "Reading (" << k << "/" << argc-1 << ") " << inputfilename << endl;

        lofasm::Lofasm_FHDR hdr(buffer);
        //hdr.print();
        int nerr = 0;
        unsigned int N_contiguous = 0;
        unsigned int N_bursts = 0; // count total number of valid bursts
        bool corrupt_break = false; // set to true if break due to corruption
        double start_mjd = hdr.mjd; // mjd of the 1st valid integration in the file
        double start_burst_id = 0;
        bool newBlock = true; // start a new contiguous block (file)
        double dataread_start = get_time();
        vector<vector<double>>* pv;
        double* pdata;
        vector<vector<double>> v;

        /*
          Open output DAT files.
          Independently write  data files and then concatenate at the end.
        */
        vector<ofstream*> datFiles(10); // one for each polarization
        string ofname;

        pv = new vector<vector<double>>(10);
        gzread(infile, buffer,lofasm::BURST_SIZE);  // read first burst into buffer.
        while(!gzeof(infile)){
            lofasm::LofasmSubintBurst burst(buffer);
            if (!burst.isValid()){
                /*
                  burst is not valid... either by corruption or burst interruption/lost packets
                */
                nerr++;
                if (burst.getInterrupt() != -1){
                    // move file location to start of valid burst header packet
                    // and continue reading data at the new position
                    int offset = (lofasm::PACKETS_PER_BURST - (burst.getInterrupt()/lofasm::PACKET_SIZE))*lofasm::PACKET_SIZE;
                    cout << "Interrupted Burst Buffer: correcting file location from ";
                    cout << gztell(infile)-lofasm::BURST_SIZE << " to " << gztell(infile)-offset << endl;
                    gzseek(infile, -1*offset, SEEK_CUR);
                    gzread(infile, buffer, lofasm::BURST_SIZE);

                    if(N_contiguous != 0){ // burst was interrupted in middle of a block
                        cout << "Dumping block mjd=" << hdr.mjd << " Nblocks=";
                        cout << N_contiguous << endl;
                        hdr.num_samples = N_contiguous;
                        dumpBlock(hdr, datFiles); // closes datFiles file objects
                        newBlock = true;
                    }

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
                    corrupt_break = true;
                    break; // out of while loop to skip the rest of the current file
                }
            }
            /*
              burst is valid
            */
            if (newBlock){ // first burst in block
                if (start_burst_id == 0){ // first burst in file
                    start_burst_id = burst.getId();
                }
                // calculate start mjd of this block
                int delta_id = burst.getId() - start_burst_id;
                double new_mjd = start_mjd + (hdr.int_time*delta_id)/86400.0;
                hdr.mjd = new_mjd;
                newBlock = false; // reset newBlock trigger
                N_contiguous = 0;
                // open new set of files
                for(int i=0; i<lofasm::POLS.size(); ++i){
                    pt::ptime tstamp = mjd2gregorian(hdr.mjd);
                    ofname = rootDir + lofasm::POLS[i] + "/";
                    ofname += gregorian2string(tstamp) + '_' + lofasm::POLS[i];
                    // open dat files
                    ofstream* dat_of = new ofstream;
                    dat_of->open(ofname + ".dat", ios::out | ios::binary);
                    if (!dat_of->is_open()){
                        cout << "Unable to open " << ofname+".dat" << endl;
                    }
                    datFiles[i] = dat_of;
                }
            }
            N_contiguous++; // increment contiguous burst counter
            N_bursts++; // increment total number of valid bursts
            v = *pv;
            burst.parse(v);

            // write pol data to dat files
            for (int i=0; i<v.size(); ++i){
                pdata = v[i].data(); // get pointer to data array
                datFiles[i]->write((char*) pdata, sizeof(double)*v[i].size());
            }
            // load next burst into buffer
            gzread(infile, buffer, lofasm::BURST_SIZE);
        }

        cout << "Wrapping up with " << N_contiguous << " integrations @ ";
        cout << gregorian2string(mjd2gregorian(hdr.mjd)) << endl;
        hdr.num_samples = N_contiguous; // update hdr object with number of samples parsed
        dumpBlock(hdr, datFiles);
        delete pv;
        double dataread_end = get_time();
        printf("Number of interrupts: %d\n", nerr);
        printf("Read and parsed data in %f s.\n", dataread_end-dataread_start);
        // free buffers & close files
        free(buffer);
        //for (int i=0; i<outFiles.size(); ++i) gzclose(outFiles[i]);
        gzclose(infile);
    }

    // print debug info

    return 0;
}

string createHeaderBBX(lofasm::Lofasm_FHDR& hdr, int chanID){
    //
    //  write BBX header to file.
    //
    string x, chanLabel;
    const char* pol = lofasm::POLS[chanID];
    x = "%\002BX\n"; // BBX signature "^BBX"
    x += "%hdr_type: LoFASM-filterbank\n";
    x += "%hdr_version: 1\n"; // THIS SHOULD BE CONFIGURABLE!
    x += "%station: " + to_string(hdr.station_id) + "\n";
    x += "%channel: " + (string) pol + "\n";
    x += "%channel_label: " + hdr.get_channel_label(pol) + "\n";
    x += "%start_mjd: " + to_string(hdr.mjd) + "\n";
    x += "%time_offset_J2000: 0 (s)\n";
    x += "%frequency_offset_DC: 0 (Hz)\n";
    x += "%dim1_label: time (s)\n";
    x += "%dim1_start: " + to_string((hdr.mjd - (2451545 - 2400000.5))*86400) + "\n";
    x += "%dim1_span: " + to_string(hdr.int_time*hdr.num_samples) + "\n";
    x += "%dim2_label: frequency (Hz)\n";
    x += "%dim2_start: " + to_string(hdr.fstart) + "\n";
    x += "%dim2_span: " + to_string(hdr.fstep*hdr.num_freq) + "\n";
    x += "%data_label: ";
    if (pol[0] == pol[1]) x += "power spectrum (arbitrary)\n";
    else x += "cross spectrum (arbitrary)\n";
    x += "%data_offset: 0\n";
    x += "%data_scale: 1\n";
    x += "%data_type: real64\n";
    x += to_string(hdr.num_samples) + " " + to_string(hdr.num_freq) + " ";
    if (pol[0] == pol[1]) x += "1 ";
    else x += "2 ";
    x += "64 raw256\n";

    return x;
}

double get_time(){
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec + (tp.tv_nsec / pow(10,9));
}

void dumpBlock(lofasm::Lofasm_FHDR& hdr, vector<ofstream*>& datFiles){
    /*
      dump files in current block.
    */
    string h, ofname;
    char* buffer = (char*) malloc(8192);
    for (int i=0; i<lofasm::POLS.size(); ++i){
        // close DAT file objects
        datFiles[i]->flush();
        datFiles[i]->close();
        delete datFiles[i]; // release ofstream pointer
        // generate fname
        ofname = rootDir + lofasm::POLS[i] + "/";
        ofname += gregorian2string(mjd2gregorian(hdr.mjd));
        ofname += (string) "_" + lofasm::POLS[i];
        // write header string to BBX file
        h = createHeaderBBX(hdr, i);
        gzFile ofile = gzopen((ofname+".bbx.gz").c_str(), "wb");
        gzwrite(ofile, h.c_str(), h.length());
        // dump data from .dat file into compressed BBX file
        ifstream datFile(ofname+".dat", ios::in | ios::binary);
        datFile.read(buffer, 8192);
        while(!datFile.eof()){
            gzwrite(ofile, buffer, 8192);
            datFile.read(buffer, 8192);
        }
        //write whatever is left over
        //gzwrite(ofile, buffer, lofasm::BURST_SIZE);
        datFile.close();
        gzclose(ofile);


        if (remove((ofname+".dat").c_str())){
            cerr << "Unable to delete " << ofname+".dat" << endl;
            perror("");
        }
    }
    free(buffer);
}

pt::ptime mjd2gregorian(double mjd){
    /*
     * calculate gregorian date from mjd value using formula from
     * https://aa.usno.navy.mil/faq/docs/JD_Formula.php
     * and the fact that 
     * MJD = JD - 2400000.5
     */
    double JD = mjd + 2400000.5;
    int jd, year, month, day, I, J, K, L, N;
    L = ( (int) JD ) + 68569;
    N = 4*L/146097;
    L = L-(146097*N+3)/4;
    I = 4000*(L+1)/1461001;
    L = L-1461*I/4+31;
    J = 80*L/2447;
    K = L-2447*J/80;
    L = J/11;
    J = J+2-12*L;
    I = 100*(N-49)+I+L;
    year = I;
    month = J;
    day = K;
    pt::ptime date(gd::date(year, month, day), pt::time_duration(12,0,0)); // integer jd starts at noon
    // calculate time since noon 
    double daypart = JD - (int) JD;
    double daypart_us = daypart * 86400000000.0;
    pt::time_duration delta = pt::microseconds((long) daypart_us);
    date += delta;
    return date;
}
string gregorian2string(pt::ptime t) {
    gd::date date = t.date(); // date portion
    auto ymd = date.year_month_day();
    pt::time_duration tod = t.time_of_day();

    stringstream ss;
    ss << ymd.year << setw(2) << setfill('0') << (int) ymd.month;
    ss << setw(2) << setfill('0') << ymd.day;
    ss << '_';
    ss << setw(2) << setfill('0') << tod.hours();
    ss << setw(2) << setfill('0') << tod.minutes();
    ss << setw(2) << setfill('0') << tod.seconds();
    return ss.str();
}

