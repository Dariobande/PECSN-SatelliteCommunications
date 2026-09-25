
#include "Oracle.h"

Define_Module(Oracle);


void Oracle::initialize()
{
    int terminalCount = getParentModule()->par("terminalCount").intValue();
    packetLocations.resize(terminalCount);
}

void Oracle::registerPacket(int terminalId, int blockIndex, int packetIndex, bool isLastSegment)
{
    packetLocations[terminalId].push_back(
        { blockIndex, packetIndex, isLastSegment }
    );

#ifdef DEBUG_TERMINALS
    EV_DEBUG << "[oracle]> Registered packet { blockIndex: " << blockIndex << ", packetIndex: "
            << packetIndex << ", isLastSegment: " << isLastSegment << "} for terminal " << terminalId << endl;
#endif
}

std::list<PacketLocation>& Oracle::getPacketLocations(int terminalId)
{
    return packetLocations[terminalId];
}
