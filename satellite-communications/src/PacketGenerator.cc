
#include "PacketGenerator.h"

Define_Module(PacketGenerator);


void PacketGenerator::initialize()
{
    /*
     * Debug signal used to plot the distribution of the arrival
     *  times of the packets modulo the communication slot duration.
     */
    debugWithinSlotArrivalTimeSignal = registerSignal("debugWithinSlotArrivalTime");
    cModule *satCom = getParentModule()->getParentModule();
    communicationSlotDurationMS =
            SimTime(satCom->par("communicationSlotDuration").doubleValue()).inUnit(SIMTIME_MS);

    meanPacketInterarrivalTime = par("meanPacketInterarrivalTime").doubleValue();
    if (meanPacketInterarrivalTime < 0.0)
    {
        throw cRuntimeError(this, "meanPacketInterarrivalTime (%f) must be a non negative number",
                meanPacketInterarrivalTime);
    }

    minPacketSize = par("minPacketSize").intValue();
    maxPacketSize = par("maxPacketSize").intValue();

    if (minPacketSize < 0)
    {
        throw cRuntimeError(this, "minPacketSize (%d) must be a non negative integer", minPacketSize);
    }

    if (minPacketSize > maxPacketSize)
    {
        throw cRuntimeError(this, "minPacketSize (%d) cannot be greater than maxPacketSize (%d)",
                minPacketSize, maxPacketSize);
    }

    /*
     * A packet with a size greater than this value will NEVER be scheduled as it exceeds the
     *  maximum possible frame size (obtained when all the blocks have the highest coding rate).
     */
    if (maxPacketSize > getMaxTheoreticalFrameBytes())
    {
        throw cRuntimeError(this, "maxPacketSize (%d) must be less than or equal to the maximum "
                "theoretical frame size %d", maxPacketSize, getMaxTheoreticalFrameBytes());
    }

    id = getIndex();

    /* A timer tick represents the arrival of a packet from the external world */
    timer = new cMessage("timer");
    double nextArrivalTime = exponential(meanPacketInterarrivalTime);
    scheduleAt(simTime() + SimTime(nextArrivalTime), timer);

#ifdef DEBUG_RNGS
    EV_DEBUG << std::fixed << std::setprecision(5) << "[" << simTime().dbl() << "]> Extracted nextArrivalTime "
            "for terminal " << id << " | Total RNG calls: " << getRNG(0)->getNumbersDrawn() << endl;
#endif

    EV_INFO << "[packetGenerator " << id << "]> The first packet is scheduled to arrive in "
            << nextArrivalTime * 1e3 << " ms" << endl;
}

void PacketGenerator::handleMessage(cMessage *msg)
{
    /*
     * Received messages are self messages (timers) by design (there are no input ports).
     * When a timer ticks, a new packet gets created and sent to the scheduler.
     */

    int byteLength = intuniform(minPacketSize, maxPacketSize);
    GenericPacket *packet = new GenericPacket("genericPacket");
    packet->setTerminalId(id);
    packet->setByteLength(byteLength);
    send(packet, "out");

    emit(debugWithinSlotArrivalTimeSignal, simTime().inUnit(SIMTIME_MS) % communicationSlotDurationMS);

#ifdef DEBUG_RNGS
    EV_DEBUG << std::fixed << std::setprecision(5) << "[" << simTime().dbl() << "]> Extracted byteLength "
                    "for terminal " << id << " | Total RNG calls: " << getRNG(0)->getNumbersDrawn() << endl;
#endif

    EV_INFO << "[packetGenerator " << id << "]> Generated packet { id: "
            << packet->getTreeId() << ", byteLength: " << byteLength << " }" << endl;

    double nextArrivalTime = exponential(meanPacketInterarrivalTime);
    scheduleAt(simTime() + SimTime(nextArrivalTime), timer);

#ifdef DEBUG_RNGS
    EV_DEBUG << std::fixed << std::setprecision(5) << "[" << simTime().dbl() << "]> Extracted nextArrivalTime "
                "for terminal " << id << " | Total RNG calls: " << getRNG(0)->getNumbersDrawn() << endl;
#endif

    EV_INFO << "[packetGenerator " << id << "]> The next packet is scheduled to arrive in "
            << nextArrivalTime * 1e3 << " ms" << endl;
}

void PacketGenerator::finish()
{
    cancelAndDelete(timer);
}
