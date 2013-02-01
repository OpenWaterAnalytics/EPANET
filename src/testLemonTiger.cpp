
#include "testLemonTiger.h"
#include "epanet2.h"


using namespace std;

void checkErr(int err, std::string function);

int main(int argc, char * argv[]) {
  
  cout << "Lemon Tiger TEST" << endl
       << "________________" << endl;
  
  
  long tstep = 0;
  long simulationTime = 0;
  long nextEventH = 0;
  long simTimeRemaining = 0;
  
  try {
    
    /*  Batch solver (old epanet) */
    cout << "*****Original EPANET results******" << endl;
    checkErr( ENopen(argv[1], argv[2], ""), "ENopen" );
    
    checkErr( ENopenH(), "ENopenH" );
    checkErr( ENinitH(EN_SAVE), "ENinitH" );
    
    do {
      /* Solve for hydraulics & advance to next time period */
      tstep = 0;
      checkErr( ENrunH(&simulationTime), "ENrunH" );
      checkErr( ENnextH(&nextEventH), "ENnextH" );
      
      // gather hydraulic results
      
    } while (nextEventH > 0);
    // hydraulics are done
    checkErr( ENcloseH(), "ENcloseH" );
    
    cout << "Running WQ..." << endl;
    
    checkErr( ENopenQ(), "ENopenQ" );
    checkErr( ENinitQ(EN_SAVE), "ENinitQ" );
    
    do {
      checkErr( ENrunQ(&simulationTime), "ENrunQ" );
      checkErr( ENstepQ(&simTimeRemaining), "ENstepQ" );
      
      // wq results
      
    } while (simTimeRemaining > 0);
    // water quality is done
    checkErr( ENcloseQ(), "ENcloseQ" );
    
    // everything is done
    checkErr( ENclose(), "ENclose" );
    
    
    nextEventH = 0;
    simTimeRemaining = 0;
    simulationTime = 0;
    
    /* stepwise solver (LemonTiger) */
    cout << "*****LemonTiger results******" << endl;
    
    checkErr( ENopen(argv[1], argv[2], NULL), "ENopen" );
    
    checkErr( ENopenH(), "ENopenH" );
    checkErr( ENinitH(EN_SAVE), "ENinitH" );
    checkErr( ENopenQ(), "ENopenQ" );
    checkErr( ENinitQ(EN_SAVE), "ENinitQ" );
    
    do {
      /* Solve for hydraulics & advance to next time period */
      tstep = 0;
      checkErr( ENrunH(&simulationTime), "ENrunH" );
      checkErr( ENnextH(&nextEventH), "ENnextH" );
      
      // hydraulic results
      
      checkErr( ENrunQ(&simulationTime), "ENrunQ" );
      checkErr( ENstepQ(&simTimeRemaining), "ENstepQ" );
      
      // wq results
      
    } while (simTimeRemaining > 0);
    // all done
    checkErr( ENcloseH(), "ENcloseH" );
    checkErr( ENcloseQ(), "ENcloseQ" );
    checkErr( ENclose(), "ENclose" );
    
    
  } catch (int err) {
    cerr << "exiting with error " << err << endl;
  }
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
