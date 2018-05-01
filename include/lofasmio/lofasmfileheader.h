// fileheader.h
// lofasmfileheader.h
/*
  ### version 5 format
  header field size: 8 bytes / ascii chars
  total header string size: 128 bytes

  1. Header Signature
  2. Header Version
  3. Header Length
  4. Station ID
  5. Number of frequency bins
  6. Frequency Start (Hz)
  7. Frequency Step (Hz, fractional digits; 0.XXXXXXXX)
  8. MJD Day (integer)
  9. MJD milliseconds since start of day
  10. Integration time (s)
  11. Data Format Version
  12. Number of Time Samples being written to disk
  13. Trunk Line A Mapping Code
  14. Trunk Line B Mapping Code
  15. Trunk Line C Mapping Code
  16. Trunk Line D Mapping Code
*/

#ifndef LOFASMFILEHEADER_H
#include <string>
namespace lofasm{
const unsigned int FHDR_ENTRY_LENGTH_BYTES = 8;
const unsigned int FHDRV5_LENGTH_BYTES = 128;
typedef char HDR_ENTRY[FHDR_ENTRY_LENGTH_BYTES+1];

class Lofasm_FHDR {
 public:
    HDR_ENTRY sig,
        trunkA,
        trunkB,
        trunkC,
        trunkD;
    int version,
        length,
        station_id,
        num_freq,
        mjd_day,
        dfmt_version,
        num_samples;
    double fstart,
        mjd_ms,
        int_time,
        mjd,
        fstep;

    Lofasm_FHDR(char*);
    Lofasm_FHDR();

    void load(char*);
    void print();
    std::string get_channel_label(const char*) const;
};
}
#define LOFASMFILEHEADER_H
#endif
