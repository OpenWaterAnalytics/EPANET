//
// test_node.cpp
//
// Date Created: February 8, 2019
//
// Author: Michael E. Tryby
//         US EPA - ORD/NRMRL
//


#define BOOST_TEST_MODULE "node"

#include "test_shared.hpp"


using namespace std;
using namespace boost;



BOOST_AUTO_TEST_SUITE (test_node)


BOOST_FIXTURE_TEST_CASE(test_node_getvalue, FixtureOpenClose)
{
    const auto junc_props = {
        EN_ELEVATION,
        EN_BASEDEMAND,
        EN_PATTERN,
        EN_EMITTER,
        EN_INITQUAL,
        //demand
        //head
        //pressure
        //quality
    };
    const int num_props = 5;

    std::vector<double> test (num_props);
    double *value = test.data();

    std::vector<double> ref = {710.0, 150.0, 1.0, 0.0, 0.5};


    for (EN_NodeProperty p : junc_props) {
         error = EN_getnodevalue(ph, 2, p, value++);
         BOOST_REQUIRE(error == 0);
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(ref.begin(), ref.end(), test.begin(), test.end());


    const auto tank_props = {
        EN_ELEVATION,
        EN_INITQUAL,
        EN_TANKLEVEL,
        EN_INITVOLUME,
        EN_MIXMODEL,
        EN_MIXZONEVOL,
        //demand
        //head
        //pressure
        //quality
        EN_TANKDIAM,
        EN_MINVOLUME,
        EN_MAXVOLUME,
        EN_VOLCURVE,
        EN_MINLEVEL,
        EN_MAXLEVEL,
        EN_MIXFRACTION,
        EN_TANK_KBULK,
        EN_TANKVOLUME
    };

}


BOOST_AUTO_TEST_SUITE_END()
