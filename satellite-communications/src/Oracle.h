
#ifndef ORACLE_H_
#define ORACLE_H_

#include <vector>
#include <list>

#include "omnetpp.h"

using namespace omnetpp;

struct PacketLocation
{
    int blockIndex;
    int packetIndex;
    /*
     * Packets may be segmented into multiple blocks.
     * This boolean allows a terminal reading its packets to emit
     *  a delay statistic only when the whole packet is fetched.
     */
    bool isLastSegment;
};

class Oracle : public cSimpleModule
{
    std::vector<std::list<PacketLocation>> packetLocations;

protected:
    virtual void initialize() override;

public:
    void registerPacket(int terminalId, int blockIndex, int packetIndex, bool isLastSegment = true);
    std::list<PacketLocation>& getPacketLocations(int terminalId);
};

#endif /* ORACLE_H_ */
