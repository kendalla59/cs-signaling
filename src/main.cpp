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
#include "system.h"
#include "config.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <vector>

using rrsim::EdgePtr;
using rrsim::Node;
using rrsim::TrainPtr;

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
    EdgePtr eptr = sys().createEdge();
    if (!eptr) {
        std::cout << "ERROR: Failed to add new track segment" << std::endl;
        return EFAULT;
    }
    std::cout << "Added new track segment \""
              << eptr->name() << "\"" << std::endl;
    return 0;
}

static int cmdConnectSegments()
{
    std::string resp1 = enterName();
    if (resp1.empty()) { return 0; }
    EdgePtr eptr1 = sys().getEdge(resp1);
    if (!eptr1) {
        std::string rnum = nameFromNumber(resp1);
        eptr1 = sys().getEdge(rnum);
        if (!eptr1) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
        resp1 = rnum;
    }
    rrsim::eEnd end1 = enterAorB();

    std::string resp2 = enterName();
    if (resp2.empty()) { return 0; }
    EdgePtr eptr2 = sys().getEdge(resp2);
    if (!eptr2) {
        std::string rnum = nameFromNumber(resp2);
        eptr2 = sys().getEdge(rnum);
        if (!eptr2) {
            std::cout << "No such segment \"" << resp2 << "\"" << std::endl;
            return EINVAL;
        }
        resp2 = rnum;
    }
    rrsim::eEnd end2 = enterAorB();

    try {
        rrsim::EdgeEnd seg1(eptr1, end1);
        rrsim::EdgeEnd seg2(eptr2, end2);
        sys().connectSegments(seg1, seg2);
        eptr1->show();
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
    EdgePtr eptr = sys().getEdge(resp1);
    if (!eptr) {
        std::string rnum = nameFromNumber(resp1);
        eptr = sys().getEdge(rnum);
        if (!eptr) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
    }
    rrsim::eEnd end1 = enterAorB();

    try {
        eptr->placeSignalLight(end1);
        sys().updateAllSignals();
        eptr->show();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdSignalAllJunctions()
{
    try {
        sys().addSignalsToAllJunctions();
        sys().updateAllSignals();
    }
    catch (std::exception & ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdToggleSwitch()
{
    rrsim::NodeVec jctv = sys().getAllJunctions();
    if (jctv.empty()) {
        std::cout << ">>> There are no junctions in the track network <<<"
                  << std::endl;
        return 0;
    }
    int jnum = 0;
    for (auto it: jctv) {
        std::cout << ++jnum << ": " << it->name() << std::endl;
    }
    std::cout << "Enter junction (1.." << jnum << "): ";
    std::string numstr;
    std::getline(std::cin, numstr);
    if (numstr.empty()) {
        std::cout << "No entry, quitting function..." << std::endl;
        return 0;
    }
    int val = std::stoi(numstr);
    if ((val < 1) || (val > jnum)) {
        std::cout << "No such junction" << std::endl;
        return EINVAL;
    }
    val--; // Make the index zero based.
    jctv[val]->toggleSwitchPos();
    std::cout << jctv[val]->name() << ": junction switch is ";
    rrsim::eJSwitch jsw = jctv[val]->getSwitchPos();
    std::cout << ((jsw == rrsim::eSwitchLeft) ? "LEFT" : "RIGHT" ) << std::endl;

    sys().updateAllSignals();
    return 0;

}

static int cmdListSegments()
{
    return sys().showEdges();
}

static int cmdShowConnections()
{
    return sys().showNodes();
}

static int cmdPlaceTrain()
{
    TrainPtr tptr;
    std::cout << "Enter train name (RETURN to create new): ";
    std::string resp;
    std::getline(std::cin, resp);
    if (resp.empty()) {
        tptr = sys().createTrain();
        std::cout << "Placing new train \""
                  << tptr->name() << "\":" << std::endl;
    }
    else {
        tptr = sys().getTrain(resp);
        if (!tptr) {
            std::cout << "No such train: \"" << resp << "\"" << std::endl;
            return EINVAL;
        }
    }
    std::cout << "Starting - ";
    std::string resp1 = enterName();
    if (resp1.empty()) { return 0; }
    EdgePtr eptr1 = sys().getEdge(resp1);
    if (!eptr1) {
        std::string rnum = nameFromNumber(resp1);
        eptr1 = sys().getEdge(rnum);
        if (!eptr1) {
            std::cout << "No such segment \"" << resp1 << "\"" << std::endl;
            return EINVAL;
        }
    }
    std::cout << "Ending - ";
    std::string resp2 = enterName();
    if (resp2.empty()) { return 0; }
    EdgePtr eptr2 = sys().getEdge(resp2);
    if (!eptr2) {
        std::string rnum = nameFromNumber(resp2);
        eptr2 = sys().getEdge(rnum);
        if (!eptr2) {
            std::cout << "No such segment \"" << resp2 << "\"" << std::endl;
            return EINVAL;
        }
    }

    try {
        rrsim::EdgeEnd edge = tptr->getPosition();
        EdgePtr eptr = edge.eeEdge.lock();
        if (eptr) { eptr->setTrain(nullptr); }
        tptr->placeOnTrack(eptr1, eptr2);
        sys().updateAllSignals();
        tptr->show();
    }
    catch (std::exception& ex) {
        std::cout << "ERROR: " << ex.what() << std::endl;
        return EFAULT;
    }

    return 0;
}

static int cmdStepSimulation()
{
    return sys().stepSimulation();
}

static int cmdRunSimulation()
{
    return sys().runSimulation();
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
    int rc = sys().serialize(ofstr);
    ofstr.close();
    return rc;
}

static int cmdLoadNetwork()
{
    if (sys().edgeCount() != 0) {
        std::cout << "WARNING: This will delete the existing network" << std::endl;
        std::cout << "         Press RETURN key at the prompt to quit" << std::endl;
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

    int rc = sys().deserialize(ifstr);
    ifstr.close();
    return rc;
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
            "8. Add Signals To All Junctions"               << std::endl <<
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
    case 8:
        std::cout << "---------- Place Signals On All Junctions ----------" << std::endl;
        rc = cmdSignalAllJunctions();
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
            "5. [S]tep the train simulation"                << std::endl <<
            "6. [R]un the train simulation"                 << std::endl <<
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
    if (resp == "S" || resp == "s") { cmd = 5; }        // Special case for "step".
    else if (resp == "R" || resp == "r") { cmd = 6; }   // Special case for "run".
    else { try { cmd = std::stoi(resp); } catch (...) { cmd = 0; } }

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
    case 6:
        std::cout << "------------------ Run Simulation ------------------" << std::endl;
        rc = cmdRunSimulation();
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

    sys().resetTrackNetwork();
    return 0;
}
