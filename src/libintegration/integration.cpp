// integration.cpp
#include <cstring>
#include <cstdlib>
#include "lofasmio/integration.h"
#include "lofasmio/parse_raw.h"
#include "lofasmio/lofasmfileheader.h"
#include <iostream>
#include <string>
#include <vector>

/*
  lofasm::SubIntegration definitions
*/
using namespace std;
using namespace lofasm;

lofasm::SubIntegration::SubIntegration(double fstart,
                                       double fstep,
                                       double mjd,
                                       int station_id,
                                       int N,
                                       double* data){
    this->fstart = fstart;
    this->fstep = fstep;
    this->mjd = mjd;
    this->station_id = station_id;
    this->Nfreq = N;
    this->data = new double[Nfreq*sizeof(int)];
    std::memcpy(this->data, data, N);
}

int lofasm::SubIntegration::size(){
    return this->Nfreq;
}

lofasm::SubIntegration::~SubIntegration(){
    delete[] this->data;
}

/*
  lofasm::LofasmSubintBurst definitions
*/
lofasm::LofasmSubintBurst::LofasmSubintBurst(){};
lofasm::LofasmSubintBurst::LofasmSubintBurst(const char* inbuf){setInputBuffer(inbuf);}
bool lofasm::LofasmSubintBurst::validate(){
    /*
      valid input buffer by ensuring that the burst signature
      _only_ shows up in the header packet.
    */
    valid = true; // assume all is good at first
    const char* data = this->rawdata;

    // check the top of the header packet
    unsigned int testVal = binary_to_uint(data, 3);
    if (testVal != BURST_KEY){
        valid = false;
    }

    /*
      Check the top of the remaining data packets.
      If one of the "data packets" turns out to contain the
      Burst Key, then the current packet is corrupted by interruption.

      Set interrupt to location of interruption. This will be the number
      of bytes since the beginning of the buffer.
    */

    for (int i=1; i<17; ++i){
        data += PACKET_SIZE;
        testVal = binary_to_uint(data, 3);
        if (testVal == BURST_KEY){
            valid = false;
            interrupt = i*PACKET_SIZE;
            return valid;
        }
    }

    /*
      if valid is still false here then BURST_KEY wasn't found at the top of
      any of the packets in the burst buffer.
      File is most likely corrupt.
    */
    interrupt = -1;
    return valid;
}
void lofasm::LofasmSubintBurst::setInputBuffer(const char* inbuf){
    interrupt = (unsigned int) NULL;
    rawdata = inbuf;

    validate();

    // set signature and burst id
    id = binary_to_uint(rawdata+3, 3);
    sig = binary_to_uint(rawdata+6,3);
}
unsigned int lofasm::LofasmSubintBurst::getId() const{return id;}
unsigned int lofasm::LofasmSubintBurst::getSig() const{return sig;}
unsigned int lofasm::LofasmSubintBurst::getInterrupt() const{return interrupt;}
bool lofasm::LofasmSubintBurst::isValid() const{return valid;}
void lofasm::LofasmSubintBurst::parse(std::vector<std::vector<double>>& poldata) const{
    /*
      Prepare array to hold all 10 polarizations.
      Each with 1024 frequency channels.
      The auto correlation arrays will have 1024 elements.
      The cross correlation arrays will have 2048 elements.

      Order of polarizations:
      AA, BB, CC, DD, AB, AC, AD, BC, BD, CD
    */

    // ensure that buffer is valid
    if (!valid){return;}

    int nPols = 10;
    int nFreqs = 1024;

    //std::vector<std::vector<double>>* pols = new std::vector<std::vector<double>>(10);

    // parse autocorrelations
    parseSubintReal(poldata[0], poldata[1],
                    rawdata+PACKET_SIZE, rawdata+9*PACKET_SIZE);
    parseSubintReal(poldata[2], poldata[3],
                    rawdata+2*PACKET_SIZE, rawdata+10*PACKET_SIZE);

    // parse cross correlations
    for (int i=4; i<10; ++i){
        parseSubintComplex(poldata[i], rawdata+(i-1)*PACKET_SIZE,
                           rawdata+(i+7)*PACKET_SIZE);
    }
}
void lofasm::LofasmSubintBurst::parseSubintReal(std::vector<double>& X,
                                                std::vector<double>& Y,
                                                const char* ebuf,
                                                const char* obuf) const{
    for (int i=0; i<512; ++i){
        X.push_back((double) binary_to_uint(ebuf, 4));
        X.push_back((double) binary_to_uint(obuf, 4));
        Y.push_back((double) binary_to_uint(ebuf+4, 4));
        Y.push_back((double) binary_to_uint(obuf+4, 4));

        // update pointers
        ebuf += 8;
        obuf += 8;
    }
}

void lofasm::LofasmSubintBurst::parseSubintComplex(std::vector<double>& X,
                                                   const char* ebuf,
                                                   const char* obuf) const{
    for (int i=0; i<512; ++i){
        X.push_back((double) binary_to_int(ebuf, 4));
        X.push_back((double) binary_to_int(ebuf+4,4));
        X.push_back((double) binary_to_int(obuf, 4));
        X.push_back((double) binary_to_int(obuf+4,4));
        // update pointers
        ebuf += 8;
        obuf += 8;
    }
}
lofasm::LofasmSubintBurst::~LofasmSubintBurst(){}
