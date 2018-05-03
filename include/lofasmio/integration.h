// integration.h

/*
  Data structures for reading raw lofasm data
*/
#ifndef INTEGRATION_H
#include "lofasmfileheader.h"
#include <cstdlib>
#include <string>
#include <vector>

namespace lofasm {
    /*
      A BURST is a contiguous sequence of 17 network packets.
      Each network packet has a length of 8192 Bytes.
    */
    const int BURST_SIZE = 139264;
    const int BURST_KEY = 14613675; // \xde\xfc\xab
    const int PACKET_SIZE = 8192;
    const int PACKETS_PER_BURST = 17;
    const std::vector<const char*> POLS = {"AA", "BB", "CC", "DD", "AB", "AC", "AD",
                                           "BC", "BD", "CD"};

    struct SubIntegration{
        double fstart, fstep, mjd;
        int station_id, Nfreq;
        double* data;
        // methods
        SubIntegration(double, double, double, int, int, double*);
        int size(); // return size of subint
        ~SubIntegration();
    };

    class LofasmSubintBurst{
    public:
        LofasmSubintBurst(const char*);
        unsigned int getId() const;
        unsigned int getSig() const;
        unsigned int getInterrupt() const;
        bool isValid() const;
        void parse(std::vector<std::vector<double>>&) const;
        ~LofasmSubintBurst();
    private:
        const char* rawdata; // raw data
        std::vector<std::vector<double>>* data; // parsed data
        unsigned int id; // subint burst id number
        unsigned int sig; // subint burst signature
        bool valid;
        int interrupt; // location of interrupting burst. -1 if invalidated at 0
        // private methods
        bool validate();
        void parseSubintReal(std::vector<double>&, std::vector<double>&, const char*, const char*) const;
        void parseSubintComplex(std::vector<double>&, const char*, const char*) const;
    };




} // end namespace
#define INTEGRATION_H
#endif
