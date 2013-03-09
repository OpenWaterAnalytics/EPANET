#include <map>
#include <iomanip>
#include <math.h>
#include "testLemonTiger.h"
#include "toolkit.h"

#define COLW 15
#define OUTPRECISION 6

using namespace std;

typedef struct {
  double head;
  double demand;
  double quality;
} nodeState_t;

typedef struct {
  double flow;
} linkState_t;

typedef map<int, nodeState_t> networkNodeState_t; // nodeIndex, state
typedef map<int, linkState_t> networkLinkState_t; // linkIndex, state

typedef struct {
  networkNodeState_t nodeState;
  networkLinkState_t linkState;
} networkState_t;
typedef map<long, networkState_t> result_t;     // time, networkState
// access results by, for instance, resultsContainer[time][nodeIndex].head



void checkErr(int err, std::string function);
void saveHydResults(networkState_t* networkState);
void saveQualResults(networkState_t* networkState);
void printResults(result_t* state1, result_t* state2, std::ostream& out);
void compare(result_t* results1, result_t* results2, std::ostream &out);

int main(int argc, char * argv[]) {
  
  // create storage structures for results.
  result_t epanetResults, lemonTigerResults;
  
  cout << "Lemon Tiger TEST" << endl
       << "________________" << endl;
  
  
  long simulationTime = 0;
  long nextEventH = 0, nextEventQ = 0;
  long simTimeRemaining = 0;
  
  try {
    
    /*  Batch solver (old epanet) */
    cout << "*****Original EPANET results******" << endl;
    checkErr( ENopen(argv[1], argv[2], (char*)""), "ENopen" );
    
    checkErr( ENopenH(), "ENopenH" );
    checkErr( ENinitH(EN_SAVE), "ENinitH" );
    
    cout << "Running hydraulics..." << endl;
    do {
      
      /* Solve for hydraulics & advance to next time period */      
      checkErr( ENrunH(&simulationTime), "ENrunH" );
      checkErr( ENnextH(&nextEventH), "ENnextH" );
      
      // gather hydraulic results
      saveHydResults(&epanetResults[simulationTime]);
      
      
      
    } while (nextEventH > 0);
    // hydraulics are done
    checkErr( ENcloseH(), "ENcloseH" );
    cout << "\t\t\tdone." << endl;
    cout << "Running WQ..." << endl;
    
    checkErr( ENopenQ(), "ENopenQ" );
    checkErr( ENinitQ(EN_NOSAVE), "ENinitQ" );
    
    do {
      
      checkErr( ENrunQ(&simulationTime), "ENrunQ" );
      checkErr( ENnextQ(&nextEventH), "ENstepQ" );
      
      // gather quality results
      saveQualResults(&epanetResults[simulationTime]);
      
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
    
    checkErr( ENopen(argv[1], argv[2], (char*)""), "ENopen" );
    
    checkErr( ENopenH(), "ENopenH" );
    checkErr( ENinitH(EN_NOSAVE), "ENinitH" );
    checkErr( ENopenQ(), "ENopenQ" );
    checkErr( ENinitQ(EN_NOSAVE), "ENinitQ" );
    
    cout << "Running stepwise hydraulics and water quality..." << endl;
    do {
      /* Solve for hydraulics & advance to next time period */      
      checkErr( ENrunH(&simulationTime), "ENrunH" );
      checkErr( ENrunQ(&simulationTime), "ENrunQ" );
      
      checkErr( ENnextH(&nextEventH), "ENnextH" );
      checkErr( ENnextQ(&nextEventQ), "ENstepQ" );
      
      
      saveHydResults(&lemonTigerResults[simulationTime]);
      saveQualResults(&lemonTigerResults[simulationTime]);

      
    } while (nextEventH > 0);
    cout << "\t\t\tdone." << endl;
    
    // all done
    checkErr( ENcloseH(), "ENcloseH" );
    checkErr( ENcloseQ(), "ENcloseQ" );
    checkErr( ENclose(), "ENclose" );
    
    
    // summarize the results
    //printResults(&epanetResults, &lemonTigerResults, cout);
    compare(&epanetResults, &lemonTigerResults, cout);
    
  } catch (int err) {
    cerr << "exiting with error " << err << endl;
  }
}


void saveHydResults(networkState_t* networkState) {
  int nNodes, nLinks;
  float head, demand, flow;
  ENgetcount(EN_NODECOUNT, &nNodes);
  ENgetcount(EN_LINKCOUNT, &nLinks);
  for (int iNode = 1; iNode <= nNodes; ++iNode) {
    ENgetnodevalue(iNode, EN_HEAD, &head);
    ENgetnodevalue(iNode, EN_DEMAND, &demand);
    (*networkState).nodeState[iNode].head = head;
    (*networkState).nodeState[iNode].demand = demand;
  }
  for (int iLink = 1; iLink <= nLinks; ++iLink) {
    ENgetlinkvalue(iLink, EN_FLOW, &flow);
    (*networkState).linkState[iLink].flow = flow;
  }
}


void saveQualResults(networkState_t* networkState) {
  int nNodes;
  float quality;
  ENgetcount(EN_NODECOUNT, &nNodes);
  
  for (int iNode = 1; iNode <= nNodes; iNode++) {
    ENgetnodevalue(iNode, EN_QUALITY, &quality);
    (*networkState).nodeState[iNode].quality = quality;
  }
}


void printResults(result_t* results1, result_t* results2, std::ostream &out) {
  
  result_t::const_iterator resultIterator;
  
  for (resultIterator = (*results1).begin(); resultIterator != (*results1).end(); ++resultIterator) {
    // get the current frame
    const long time = resultIterator->first;
    const networkNodeState_t nodeState1 = resultIterator->second.nodeState;
    const networkLinkState_t linkState1 = resultIterator->second.linkState;
    
    // see if this time is indexed in the second state container
    if ((*results2).find(time) == (*results2).end()) {
      // nope.
      out << "time " << time << " not found in second result set" << endl;
    }
    else {
      // get the second result set's state
      const networkNodeState_t networkNodeState2 = (*results2)[time].nodeState;
      const networkLinkState_t networkLinkState2 = (*results2)[time].linkState;
      // print the current simulation time
      out << left;
      out << setfill('*') << setw(100) << "*" << endl;
      out << setfill(' ');
      out << setw(4) << "T = " << setw(6) << time;
      out << "|" << setw(3*COLW) << "EPANET";
      out << "|" << setw(3*COLW) << "LemonTiger" << endl;
      out << setw(10) << "Index" << "|";
      out << setw(COLW) << "Demand" << setw(COLW) << "Head" << setw(COLW) << "Quality" << "|";
      out << setw(COLW) << "Demand" << setw(COLW) << "Head" << setw(COLW) << "Quality" << endl;
      out << setprecision(OUTPRECISION);
      
      // loop through the nodes in the networkState objs, and print out the results for this time period
      networkNodeState_t::const_iterator networkNodeIterator;
      for (networkNodeIterator = nodeState1.begin(); networkNodeIterator != nodeState1.end(); ++networkNodeIterator) {
        int nodeIndex = networkNodeIterator->first;
        // trusting that all nodes are present...
        const nodeState_t nodeState1 = networkNodeIterator->second;
        const nodeState_t nodeState2 = networkNodeState2.at(nodeIndex);
        
        if (nodeState1.quality != nodeState2.quality ) {
          // epanet
          out << setw(10) << nodeIndex << "|";
          out << setw(COLW) << nodeState1.demand;
          out << setw(COLW) << nodeState1.head;
          out << setw(COLW) << nodeState1.quality;
          
          // lemontiger
          out << "|";
          out << setw(COLW) << nodeState2.demand;
          out << setw(COLW) << nodeState2.head;
          out << setw(COLW) << nodeState2.quality;
          out << endl;
        }
      }
      
      networkLinkState_t::const_iterator networkLinkIterator;
      for (networkLinkIterator = linkState1.begin(); networkLinkIterator != linkState1.end(); ++networkLinkIterator) {
        int linkIndex = networkLinkIterator->first;
        // trusting that all nodes are present...
        const linkState_t linkState1 = networkLinkIterator->second;
        const linkState_t linkState2 = networkLinkState2.at(linkIndex);
        
        if ( linkState1.flow != linkState2.flow ) {
                 // epanet
          out << setw(10) << linkIndex << "|";
          out << setw(COLW) << linkState1.flow;
          
          // lemontiger
          out << "|";
          out << setw(COLW) << linkState2.flow;
          out << endl;
        }
      }

      
    }
  }
  
}



void compare(result_t* results1, result_t* results2, std::ostream &out) {
  
  double sumHeadDiff=0, sumDemandDiff=0, sumQualDiff=0, sumFlowDiff=0;
  
  result_t::const_iterator resultIterator;
  
  for (resultIterator = (*results1).begin(); resultIterator != (*results1).end(); ++resultIterator) {
    // get the current frame
    const long time = resultIterator->first;
    const networkNodeState_t nodeState1 = resultIterator->second.nodeState;
    const networkLinkState_t linkState1 = resultIterator->second.linkState;
    
    // see if this time is indexed in the second state container
    if ((*results2).find(time) == (*results2).end()) {
      // nope.
      out << "time " << time << " not found in second result set" << endl;
    }
    else {
      // get the second result set's state
      const networkNodeState_t networkNodeState2 = (*results2)[time].nodeState;
      const networkLinkState_t networkLinkState2 = (*results2)[time].linkState;
      double qualD=0;
      
      networkNodeState_t::const_iterator networkNodeIterator;
      for (networkNodeIterator = nodeState1.begin(); networkNodeIterator != nodeState1.end(); ++networkNodeIterator) {
        int nodeIndex = networkNodeIterator->first;
        // trusting that all nodes are present...
        const nodeState_t nodeState1 = networkNodeIterator->second;
        const nodeState_t nodeState2 = networkNodeState2.at(nodeIndex);
        
        sumHeadDiff += fabs(nodeState1.head - nodeState2.head);
        sumDemandDiff += fabs(nodeState1.demand - nodeState2.demand);
        
        qualD += fabs(nodeState1.quality - nodeState2.quality);
      }
      //out << "T: " << time << " dq: " << setprecision(20) << qualD << endl;
      sumQualDiff += qualD;
      
      networkLinkState_t::const_iterator networkLinkIterator;
      for (networkLinkIterator = linkState1.begin(); networkLinkIterator != linkState1.end(); ++networkLinkIterator) {
        int linkIndex = networkLinkIterator->first;
        // trusting that all nodes are present...
        const linkState_t linkState1 = networkLinkIterator->second;
        const linkState_t linkState2 = networkLinkState2.at(linkIndex);
        
        sumFlowDiff += fabs(linkState1.flow - linkState2.flow);
      }
    }
  }
  
  int c1 = 18;
  int p = 20;
  out << setw(c1) << "Head Diff:" << setprecision(p) << sumHeadDiff << endl;
  out << setw(c1) << "Demand Diff:" << setprecision(p) << sumDemandDiff << endl;
  out << setw(c1) << "Quality Diff:" << setprecision(p) << sumQualDiff << endl;
  out << setw(c1) << "Flow Diff:" << setprecision(p) << sumFlowDiff << endl;
  
  
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
