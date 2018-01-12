
#include <stdlib.h>
#include <cstring>

#include "epanet2.h"

// NOTE: Project Home needs to be updated to run unit test
#define PROJECT_HOME "C:/Users/mtryby/Workspace/GitRepo/michaeltryby/epanet"
#define DATA_PATH "/tools/epanet-toolkit/test/data/"

using namespace std;


int main(int argc, char *argv[])
{
    int error = 0;
    EN_ProjectHandle project = NULL;

    char inputFile[10] = "Net1.inp";
    char reportFile[10] = "net1.rpt";
    char outputFile[10] = "net1.out";

    error = EN_alloc(&project);
    error = EN_open(project, inputFile, reportFile, outputFile);

    error = EN_solveH(project);
    error = EN_solveQ(project);
    error = EN_report(project);

    error = EN_close(project);
    error = EN_free(project);

    return error;
}
