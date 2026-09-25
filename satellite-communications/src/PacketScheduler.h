
#ifndef PACKETSCHEDULER_H_
#define PACKETSCHEDULER_H_

#include <vector>
#include <algorithm>
#include <iomanip>

#include "omnetpp.h"
#include "Oracle.h"
#include "CodingRatePacket_m.h"
#include "Frame_m.h"

using namespace omnetpp;

struct TerminalDescriptor
{
    int id;
    CODING_RATE codingRate;
    cQueue packetQueue;
};

class PacketScheduler : public cSimpleModule
{
    Oracle *oracle;
    cModule *satellite;
    simsignal_t throughputSignal, frameUtilizationSignal;
    simsignal_t debugInstantaneousThroughputSignal, debugFrameSizeSignal;
    simtime_t communicationSlotDuration;
    std::vector<TerminalDescriptor> terminals;
    std::vector<TerminalDescriptor*> sortedTerminals;
    int terminalCount;
    int receivedCodingRateCount;
    int blocksPerFrame;
    long totalBitsSent;

    void handlePacket(GenericPacket *packet);
    void handleCodingRatePacket(CodingRatePacket *codingRatePacket);
    void maxCRScheduling();
    const int maxBytesInBlock(CODING_RATE codingRate) const;
    void initBlock(Block *block, CODING_RATE codingRate, bool isForNewPacket, int currentBlockIndex);
    bool canSchedule(TerminalDescriptor *terminal, Block *block, int currentBlockIndex);
    bool fits(GenericPacket *packet, Frame *frame, int currentBlockIndex);
    void allocatePacketSegment(GenericPacket *packet, Frame *frame, int &lastPacketBytesLeft, int &currentBlockIndex);
    Frame *buildFrame();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif /* PACKETSCHEDULER_H_ */
