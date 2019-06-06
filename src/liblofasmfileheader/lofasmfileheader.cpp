// lofasmfileheader.cpp implementations
#include <cstring>
#include <cstdlib>
#include <iostream>
#include "lofasmio/lofasmfileheader.h"
#include <cmath>
#include <iomanip>
#include <string>
using namespace std;
using namespace lofasm;

Lofasm_FHDR::Lofasm_FHDR(char* buf) {load(buf);}
Lofasm_FHDR::Lofasm_FHDR(){}

void Lofasm_FHDR::load(char* hdr) {
    /*
      read header entries from buf and populate header variables.
      only reads the first 128 bytes from the buffer, regardless of the
      buffer's length.
    */

    HDR_ENTRY tmp;
    // load signature
    memcpy(sig, hdr, FHDR_ENTRY_LENGTH_BYTES);
    sig[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    // load header version
    memcpy(tmp, hdr+FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    version = std::atoi(tmp);
    // load header length
    memcpy(tmp, hdr+2*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    length = std::atoi(tmp);
    // load station id
    memcpy(tmp, hdr+3*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    station_id = std::atoi(tmp);
    // load num freq bins
    memcpy(tmp, hdr+4*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    num_freq = std::atoi(tmp) / 2;
    // load frequency start
    memcpy(tmp, hdr+5*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    fstart = std::atof(tmp)*1.0;
    // load frequency step
    memcpy(tmp, hdr+6*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    fstep = std::atof(tmp);
    fstep = fstep / pow((double) 10.0, (int) 2); // convert to Hz
    // load mjd day
    memcpy(tmp, hdr+7*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    mjd_day = std::atoi(tmp);
    // load mjd milliseconds
    memcpy(tmp, hdr+8*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    mjd_ms = std::atof(tmp);
    // load integration time
    memcpy(tmp, hdr+9*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    int_time = std::atof(tmp);
    // load data format version
    memcpy(tmp, hdr+10*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    dfmt_version = std::atoi(tmp);
    // load number of time samples
    memcpy(tmp, hdr+11*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    tmp[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    num_samples = std::atoi(tmp);
    // load trunk lines
    memcpy(trunkA, hdr+12*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    trunkA[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    memcpy(trunkB, hdr+13*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    trunkB[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    memcpy(trunkC, hdr+14*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    trunkC[FHDR_ENTRY_LENGTH_BYTES] = '\0';
    memcpy(trunkD, hdr+15*FHDR_ENTRY_LENGTH_BYTES, FHDR_ENTRY_LENGTH_BYTES);
    trunkD[FHDR_ENTRY_LENGTH_BYTES] = '\0';

    // calculate MJD
    mjd = mjd_day + mjd_ms/86400000.0;
}
void Lofasm_FHDR::print() {
    int w = 20;
    cout << "Header Signature      : " << setw(w) << sig << endl;
    cout << "Header Version        : " << setw(w) << version << endl;
    cout << "Header Length         : " << setw(w) << length << endl;
    cout << "Station ID            : " << setw(w) << station_id << endl;
    cout << "N Frequency bins      : " << setw(w) << num_freq << endl;
    cout << "Start Frequency (Hz)  : " << setw(w) << setprecision(2) << fstart << endl;
    cout << "Step Frequency (Hz)   : " << setw(w) << setprecision(15) << fstep << endl;
    cout << "MJD                   : " << setw(w) << setprecision(15) << mjd << endl;
    cout << "Integration Time (s)  : " << setw(w) << int_time << endl;
    cout << "Data Format Version   : " << setw(w) << dfmt_version << endl;
    cout << "Number of time samples: " << setw(w) << num_samples << endl;
    cout << "Trunk Line A          : " << setw(w) << trunkA << endl;
    cout << "Trunk Line B          : " << setw(w) << trunkB << endl;
    cout << "Trunk Line C          : " << setw(w) << trunkC << endl;
    cout << "Trunk Line D          : " << setw(w) << trunkD << endl;
}

string Lofasm_FHDR::get_channel_label(const char* pol) const {
    /*
      convert polarization name (AA, BB, CD, etc.) to a channel label
      (ONS, IEW, ONSxIEW, etc.)
    */
    string chanLabel;
    if (pol[0] == pol[1]){
        if (pol[0] == 'A') chanLabel = trunkA;
        else if (pol[0] == 'B') chanLabel = trunkB;
        else if (pol[0] == 'C') chanLabel = trunkC;
        else if (pol[0] == 'D') chanLabel = trunkD;
    }
    else{
        if (pol[0] == 'A') chanLabel = trunkA;
        else if (pol[0] == 'B') chanLabel = trunkB;
        else if (pol[0] == 'C') chanLabel = trunkC;
        else if (pol[0] == 'D') chanLabel = trunkD;
        chanLabel += 'x';
        if (pol[1] == 'A') chanLabel += trunkA;
        else if (pol[1] == 'B') chanLabel += trunkB;
        else if (pol[1] == 'C') chanLabel += trunkC;
        else if (pol[1] == 'D') chanLabel += trunkD;
    }
    return chanLabel;
}
