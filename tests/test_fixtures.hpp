

#include <string>

#include "epanet2_2.h"


// NOTE: Project Home needs to be updated to run unit test
#define DATA_PATH_INP "./net1.inp"
#define DATA_PATH_RPT "./test.rpt"
#define DATA_PATH_OUT "./test.out"


struct FixtureOpenClose{
    FixtureOpenClose() {
        path_inp = std::string(DATA_PATH_INP);
        path_rpt = std::string(DATA_PATH_RPT);
        path_out = std::string(DATA_PATH_OUT);

        EN_createproject(&ph);
        error = EN_open(ph, path_inp.c_str(), path_rpt.c_str(), path_out.c_str());
    }

    ~FixtureOpenClose() {
      error = EN_close(ph);
      EN_deleteproject(&ph);
  }

  std::string path_inp;
  std::string path_rpt;
  std::string path_out;

  int error;
  EN_Project ph;
};
