// main.cpp
//
// Author: Kendall Auel
// Description:
//     Entry point for the railroad signaling case study implementation.
//

#include "node.h"
#include "edge.h"
#include "train.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>

using rrsim::Node;
using rrsim::Edge;
using rrsim::Train;
using rrsim::g_nodeMap;
using rrsim::g_edgeMap;

Train g_train;

static std::string enterName()
{
    std::string resp;
    std::cout << "Enter track segment name: ";
    std::getline(std::cin, resp);
    if (resp.empty()) {
        std::cout << "No name entered, quitting..." << std::endl;
    }
    return resp;
}

static std::string nameFromNumber(const std::string& num)
{
    int val = std::atoi(num.c_str());
    std::stringstream ss;
    ss << "tseg" << std::setw(3) << std::setfill('0') << val;
    return ss.str();
}

static rrsim::eEnd enterAorB()
{
    rrsim::eEnd rval = rrsim::eNumEnds;
    std::string resp;
    while (rval == rrsim::eNumEnds) {
        std::cout << "Enter track end (A or B): ";
        std::getline(std::cin, resp);
        if      ((resp == "a") || (resp == "A")) { rval = rrsim::eEndA; }
        else if ((resp == "b") || (resp == "B")) { rval = rrsim::eEndB; }
        else if (!resp.empty()) {
            std::cout << "Expected one of [ABab], got: " << resp << std::endl;
        }
    }
    return rval;
}
static int cmdSmokeTest()
{
    return 0;
}

static int cmdAddSegment()
{
    Edge* pEdge = new Edge();
    g_edgeMap.insert(rrsim::EdgePair(pEdge->name(), pEdge));
    std::cout << "Added new track segment \"" << pEdge->name() << "\"" << std::endl;
    return 0;
}

static int cmdConnectSegments()
{
    std::string resp1 = enterName();
    if (resp1.empty()) { return 0; }
    auto iter1 = g_edgeMap.find(resp1);
    if (iter1 == g_edgeMap.end()) {
        resp1 = nameFromNumber(resp1);
        iter1 = g_edgeMap.find(resp1);
        if (iter1 == g_edgeMap.end()) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
    }
    rrsim::eEnd end1 = enterAorB();

    std::string resp2 = enterName();
    if (resp2.empty()) { return 0; }
    auto iter2 = g_edgeMap.find(resp2);
    if (iter2 == g_edgeMap.end()) {
        resp2 = nameFromNumber(resp2);
        iter2 = g_edgeMap.find(resp2);
        if (iter2 == g_edgeMap.end()) {
            std::cout << "No such segment \"" << resp2 << "\"" << std::endl;
            return EINVAL;
        }
    }
    rrsim::eEnd end2 = enterAorB();

    try {
        iter1->second->connectEdge(end1, iter2->second, end2);
        iter1->second->show(end1);
    }
    catch(std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EBUSY;
    }
    return 0;
}

static int cmdToggleSwitch()
{
    std::vector<Node *> junctions;
    for (auto it: g_nodeMap) {
        if (it.second->getNodeType() == rrsim::eJunction) {
            junctions.push_back(it.second);
        }
    }
    if (junctions.empty()) {
        std::cout << ">>> There are no junctions in the track network <<<"
                  << std::endl;
        return 0;
    }
    int jnum = 0;
    for (auto it: junctions) {
        std::cout << ++jnum << ": " << it->name() << std::endl;
    }
    std::cout << "Enter junction (1.." << jnum << "): ";
    std::string numstr;
    std::getline(std::cin, numstr);
    if (numstr.empty()) {
        std::cout << "No entry, quitting function..." << std::endl;
        return 0;
    }
    int val = std::atoi(numstr.c_str());
    if ((val < 1) || (val > jnum)) {
        std::cout << "No such junction" << std::endl;
        return EINVAL;
    }
    val--; // Make the index zero based.
    junctions[val]->toggleSwitchPos();
    std::cout << junctions[val]->name() << ": junction switch is ";
    rrsim::eJSwitch jsw = junctions[val]->getSwitchPos();
    std::cout << ((jsw == rrsim::eSwitchLeft) ? "LEFT" : "RIGHT" ) << std::endl;
    return 0;

}

static int cmdListSegments()
{
    try {
        for (auto it: g_edgeMap) {
            it.second->show();
        }
        std::cout << std::endl;
        std::cout << "TOTAL: " << g_edgeMap.size() << " track segments" << std::endl;
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

static int cmdShowConnections()
{
    try {
        for (auto it: g_nodeMap) {
            it.second->show();
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }
    return 0;
}

static int cmdPlaceTrain()
{
    std::string resp1 = enterName();
    if (resp1.empty()) { return 0; }
    auto iter1 = g_edgeMap.find(resp1);
    if (iter1 == g_edgeMap.end()) {
        resp1 = nameFromNumber(resp1);
        iter1 = g_edgeMap.find(resp1);
        if (iter1 == g_edgeMap.end()) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
    }
    rrsim::eEnd end1 = enterAorB();

    try {
        g_train.placeOnTrack(resp1, end1);
        g_train.show();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdStepSimulation()
{
    try {
        bool chk = g_train.stepSimulation();
        g_train.show();
        if (!chk) {
            std::cout << ">>> The Simulation Is Complete <<<" << std::endl;
        }
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

int runCommand()
{
    int rc;

    std::cout << std::endl <<
            "Train Signaling System Simulator"              << std::endl <<
            "1. Add a track segment"                        << std::endl <<
            "2. Connect track segments"                     << std::endl <<
            "3. Toggle Junction Switch"                     << std::endl <<
            "4. List track segments"                        << std::endl <<
            "5. Show track connections"                     << std::endl <<
            "6. Place train on a track segment"             << std::endl <<
            "7. Step the train simulation"                  << std::endl <<
            "8. Run smoke test"                             << std::endl <<
            "Q/quit/exit"                                   << std::endl;

    std::string resp;
    while (resp.empty()) {
        std::cout << "=> ";
        std::getline(std::cin, resp);
    }
    if (resp == "Q" || resp == "q" || resp == "quit" || resp == "exit") {
        return 1;
    }
    int cmd;
    try { cmd = std::stoi(resp); } catch (...) { cmd = 0; }

    switch (cmd) {
    case 1:
        std::cout << "---------------- Add Track Segment -----------------" << std::endl;
        rc = cmdAddSegment();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 2:
        std::cout << "-------------- Connect Track Segment ---------------" << std::endl;
        rc = cmdConnectSegments();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 3:
        std::cout << "-------------- Toggle Junction Switch --------------" << std::endl;
        rc = cmdToggleSwitch();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 4:
        std::cout << "--------------- List Track Segments ----------------" << std::endl;
        rc = cmdListSegments();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 5:
        std::cout << "----------------- Show Connections -----------------" << std::endl;
        rc = cmdShowConnections();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 6:
        std::cout << "--------------- Place Train On Track ---------------" << std::endl;
        rc = cmdPlaceTrain();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 7:
        std::cout << "----------------- Step Simulation ------------------" << std::endl;
        rc = cmdStepSimulation();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 8:
        std::cout << "------------------ Run Smoke Test ------------------" << std::endl;
        rc = cmdSmokeTest();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    default:
        std::cout << "Invalid entry: \"" << resp << "\"" << std::endl;
        rc = EINVAL;
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    }
    if (rc) std::cout << "(Error code " << rc << " was returned)" << std::endl;
    return 0;
}

// -----------------------------------------------------------------------------
// main -- Entry point
// -----------------------------------------------------------------------------

int main(int argc, char **argv) {
    std::cout << "Case Study Implementation -- Railroad Signaling System" << std::endl;
    std::cout << "Version " << cs_signaling_VERSION_MAJOR << "." << cs_signaling_VERSION_MINOR << std::endl;

    while (runCommand() == 0) {}
    return 0;
}
