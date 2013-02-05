
#include "testLemonTiger.h"
#include "toolkit.h"


using namespace std;

void checkErr(int err, std::string function);
void hydStats();
void qualStats();

int main(int argc, char * argv[]) {
  
  cout << "Lemon Tiger TEST" << endl
       << "________________" << endl;
  
  
  long tstep = 0;
  long simulationTime = 0;
  long nextEventH = 0, nextEventQ = 0;
  long simTimeRemaining = 0;
  
  try {
    
    /*  Batch solver (old epanet) */
    cout << "*****Original EPANET results******" << endl;
    checkErr( ENopen(argv[1], argv[2], "out.bin"), "ENopen" );
    
    checkErr( ENopenH(), "ENopenH" );
    checkErr( ENinitH(EN_SAVE), "ENinitH" );
    
    cout << "Running hydraulics..." << endl;
    do {
      
      /* Solve for hydraulics & advance to next time period */
      tstep = 0;
      
      checkErr( ENrunH(&simulationTime), "ENrunH" );
      checkErr( ENnextH(&nextEventH), "ENnextH" );
      
      hydStats();
      
      // gather hydraulic results
      
    } while (nextEventH > 0);
    // hydraulics are done
    checkErr( ENcloseH(), "ENcloseH" );
    cout << "\t\t\tdone." << endl;
    cout << "Running WQ..." << endl;
    
    checkErr( ENopenQ(), "ENopenQ" );
    checkErr( ENinitQ(EN_SAVE), "ENinitQ" );
    
    do {
      
      checkErr( ENrunQ(&simulationTime), "ENrunQ" );
      checkErr( ENnextQ(&nextEventH), "ENstepQ" );
      
      qualStats();
      
    } while (nextEventH > 0);
    // water quality is done
    checkErr( ENcloseQ(), "ENcloseQ" );
    cout << "\t\t\tdone." << endl;

    // everything is done
    checkErr( ENclose(), "ENclose" );
    
    
    nextEventH = 0;
    simTimeRemaining = 0;
    simulationTime = 0;
    
    /* stepwise solver (LemonTiger) */
    cout << "*****LemonTiger results******" << endl;
    
    checkErr( ENopen(argv[1], argv[2], (char*)"out2.bin"), "ENopen" );
    
    checkErr( ENopenH(), "ENopenH" );
    checkErr( ENinitH(EN_NOSAVE), "ENinitH" );
    checkErr( ENopenQ(), "ENopenQ" );
    checkErr( ENinitQ(EN_NOSAVE), "ENinitQ" );
    
    cout << "Running stepwise hydraulics and water quality..." << endl;
    do {
      /* Solve for hydraulics & advance to next time period */
      tstep = 0;
      
      checkErr( ENrunH(&simulationTime), "ENrunH" );
      checkErr( ENrunQ(&simulationTime), "ENrunQ" );
      
      //stats();
      
      checkErr( ENnextH(&nextEventH), "ENnextH" );
      checkErr( ENnextQ(&nextEventQ), "ENstepQ" );
      
      //hydStats();
      qualStats();
      
      // wq results
      //cout << simulationTime << "\t\t" << nextEventH << endl;
      
    } while (nextEventH > 0);
    cout << "\t\t\tdone." << endl;
    
    // all done
    checkErr( ENcloseH(), "ENcloseH" );
    checkErr( ENcloseQ(), "ENcloseQ" );
    checkErr( ENclose(), "ENclose" );
    
    
  } catch (int err) {
    cerr << "exiting with error " << err << endl;
  }
}



void hydStats() {
  long htime;
  int nodeIndex;
  float head;
  ENgettimeparam(EN_HTIME, &htime);
  ENgetnodeindex((char*)"1", &nodeIndex);
  ENgetnodevalue(nodeIndex, EN_HEAD, &head);
  cout << htime << "\t\th = " << head << endl;
}

void qualStats() {
  long htime;
  int nodeIndex;
  float quality;
  ENgettimeparam(EN_HTIME, &htime);
  ENgetnodeindex((char*)"1", &nodeIndex);
  ENgetnodevalue(nodeIndex, EN_QUALITY, &quality);
  cout << htime << "\t\tc = " << quality << endl;
}



void checkErr(int err, std::string function) {
  if (err > 0) {
    cerr << "Error in " << function << ": " << err << endl;
    char errmsg[1024];
    ENgeterror(err, errmsg, 1024);
    cerr << errmsg << endl;
    throw err;
  }
}
