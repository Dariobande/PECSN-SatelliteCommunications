
#ifndef PACKETGENERATOR_H_
#define PACKETGENERATOR_H_

#include <iomanip>

#include "omnetpp.h"
#include "GenericPacket_m.h"
#include "CodingRatePacket_m.h"

using namespace omnetpp;

class PacketGenerator : public cSimpleModule
{
    cMessage *timer;
    simsignal_t debugWithinSlotArrivalTimeSignal;
    int id;
    int minPacketSize;
    int maxPacketSize;
    double meanPacketInterarrivalTime;
    long communicationSlotDurationMS;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif /* PACKETGENERATOR_H_ */
