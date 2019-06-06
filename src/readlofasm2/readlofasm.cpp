// main.cpp for readlofasm tool

#include <iostream>
#include <cstdio>
#include <fstream>
#include "lofasmio/lofasmfileheader.h"
#include "lofasmio/integration.h"
#include "lofasmio/parse_raw.h"
#include "lofasmio/lofasmreader.h"
#include "lofasmio/time.h"
#include <zlib.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <time.h>
#include <math.h>

using namespace std;
using namespace lofasm;

int main(int argc, char** argv){
    if (argc != 2){
        cout << "Usage: " << argv[0] << " filename";
        cout << endl;
        return 0;
    }
    string fname = argv[1];
    cout << "Attempting to open " << fname << endl;
    LofasmStream lf(fname.c_str());
    return 0;
}

