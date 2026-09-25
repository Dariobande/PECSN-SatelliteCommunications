
#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <iomanip>

#include "omnetpp.h"
#include "Oracle.h"
#include "CodingRatePacket_m.h"
#include "Frame_m.h"

using namespace omnetpp;

enum CODING_RATE_DISTRIBUTION
{
    UNIFORM,
    BINOMIAL,
    NORMAL
};

class Terminal : public cSimpleModule
{
    Oracle *oracle;
    cMessage *timer;
    cModule *satellite;
    simsignal_t delaySignal, debugCodingRateDistributionSignal;
    simtime_t communicationSlotDuration;
    int id;
    int terminalCount;
    CODING_RATE_DISTRIBUTION codingRateDistribution;

    void handleTimer();
    void handleFrame(Frame *frame);

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif /* TERMINAL_H_ */
