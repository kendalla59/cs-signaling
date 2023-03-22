// main.cpp
//
// Author: Kendall Auel
// Description:
//     Entry point for the railroad signaling case study implementation.
//

#include "node.h"
#include "edge.h"
#include "rrsignal.h"
#include "train.h"
#include "config.h"
#include <iostream>
#include <fstream>
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

static int cmdPlaceSignal()
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
        iter1->second->placeSignalLight(end1);
        rrsim::RRsignal::updateAllSignals();
        iter1->second->show(end1);
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdToggleSwitch()
{
    std::vector<Node*> junctions;
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
    std::cout << "Starting - ";
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
    std::cout << "Ending - ";
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

    try {
        rrsim::EdgeEnd edge = g_train.getPosition();
        if (edge.eeEdge) { edge.eeEdge->setTrain(nullptr); }
        g_train.placeOnTrack(iter1->second, iter2->second);
        rrsim::RRsignal::updateAllSignals();
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
        rrsim::RRsignal::updateAllSignals();
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

static int cmdSaveNetwork()
{
    std::string path;
    std::cout << "Enter file path: ";
    std::getline(std::cin, path);
    if (path.empty()) {
        std::cout << "No response, quitting..." << std::endl;
        return 0;
    }
    std::ifstream ifstr(path);
    if (ifstr.good()) {
        std::cout << path << " exists. Replace contents? [y/n] ";
        std::string resp;
        std::getline(std::cin, resp);
        if ((resp != "Y") && (resp != "y")) {
            std::cout << "NOT replacing contents. quitting..." << std::endl;
            return 0;
        }
    }
    ifstr.close();
    std::ofstream ofstr(path, std::ofstream::trunc);
    if (!ofstr.good()) {
        std::cout << "Unable to open file " << path << ", quitting..." << std::endl;
        return 0;
    }
    for (auto iter: g_edgeMap) {
        Edge* edge = iter.second;
        if (edge) {
            ofstr << edge->serialize();
        }
    }
    ofstr.close();
    return 0;
}

static int cmdLoadNetwork()
{
    if (!g_edgeMap.empty()) {
        std::cout << "WARNING: This will delete the existing network" << std::endl;
        std::cout << "         Press ENTER key at the prompt to quit" << std::endl;
    }
    std::string path;
    std::cout << "Enter file path: ";
    std::getline(std::cin, path);
    if (path.empty()) {
        std::cout << "No response, quitting..." << std::endl;
        return 0;
    }
    std::ifstream ifstr(path);
    if (!ifstr.good()) {
        std::cout << path << " not found, quitting..." << std::endl;
        return ENOENT;
    }
    // Clear out the existing network.
    for (auto iter: g_edgeMap) {
        if (iter.second) { delete iter.second; }
    }
    g_edgeMap.clear();

    for (auto iter: g_nodeMap) {
        if (iter.second) { delete iter.second; }
    }
    g_nodeMap.clear();

    std::string segment;
    try {
        while (!ifstr.eof()) {
            std::getline(ifstr, segment);
            if (segment.empty()) continue;
            if (segment.substr(0, 7) != "track: ") {
                throw std::runtime_error("Serialized string preamble missing");
            }
            new Edge(segment);
        }
        rrsim::RRsignal::updateAllSignals();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        std::cout << "segment: \"" << segment << "\"" << std::endl;
        return EFAULT;
    }
    return 0;
}

int runCommandBuild()
{
    int rc;

    std::cout << std::endl <<
            "Build Track Network submenu"                   << std::endl <<
            "1. Add a track segment"                        << std::endl <<
            "2. Connect track segments"                     << std::endl <<
            "3. Place a signal light"                       << std::endl <<
            "4. Toggle junction switch"                     << std::endl <<
            "5. List track segments"                        << std::endl <<
            "6. Save track network"                         << std::endl <<
            "7. Load track network"                         << std::endl <<
            "R/return"                                      << std::endl;

    std::string resp;
    while (resp.empty()) {
        std::cout << "=> ";
        std::getline(std::cin, resp);
    }
    if (resp == "R" || resp == "r" || resp == "q" || resp == "return") {
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
        std::cout << "---------------- Place Signal Light ----------------" << std::endl;
        rc = cmdPlaceSignal();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 4:
        std::cout << "-------------- Toggle Junction Switch --------------" << std::endl;
        rc = cmdToggleSwitch();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 5:
        std::cout << "--------------- List Track Segments ----------------" << std::endl;
        rc = cmdListSegments();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 6:
        std::cout << "---------------- Save Track Network ----------------" << std::endl;
        rc = cmdSaveNetwork();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 7:
        std::cout << "---------------- Load Track Network ----------------" << std::endl;
        rc = cmdLoadNetwork();
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

int runCommand()
{
    int rc;

    std::cout << std::endl <<
            "Train Signaling System Simulator"              << std::endl <<
            "1. Build track network (submenu)"              << std::endl <<
            "2. List track segments"                        << std::endl <<
            "3. Show track connections"                     << std::endl <<
            "4. Place train on a track segment"             << std::endl <<
            "5. Step the train simulation"                  << std::endl <<
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
        std::cout << "--------------- Build Track Network ----------------" << std::endl;
        while (runCommandBuild() == 0) {}
        rc = 0;
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 2:
        std::cout << "--------------- List Track Segments ----------------" << std::endl;
        rc = cmdListSegments();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 3:
        std::cout << "----------------- Show Connections -----------------" << std::endl;
        rc = cmdShowConnections();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 4:
        std::cout << "--------------- Place Train On Track ---------------" << std::endl;
        rc = cmdPlaceTrain();
        std::cout << "----------------------------------------------------" << std::endl;
        break;
    case 5:
        std::cout << "----------------- Step Simulation ------------------" << std::endl;
        rc = cmdStepSimulation();
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
