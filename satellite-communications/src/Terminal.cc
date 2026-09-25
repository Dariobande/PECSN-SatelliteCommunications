
#include "Terminal.h"

Define_Module(Terminal);


void Terminal::initialize()
{
    cModule *satCom = getParentModule();

    terminalCount = satCom->par("terminalCount").intValue();
    satellite = satCom->getSubmodule("satellite");
    delaySignal = registerSignal("delay");
    debugCodingRateDistributionSignal = registerSignal("debugCodingRateDistribution");

    id = getIndex();
    communicationSlotDuration = SimTime(satCom->par("communicationSlotDuration").doubleValue());

    const char *codingRateDistributionString = par("codingRateDistribution").stringValue();
    if (strcmp(codingRateDistributionString, "uniform") == 0)
    {
        codingRateDistribution = UNIFORM;
    }
    else if (strcmp(codingRateDistributionString, "binomial") == 0)
    {
        codingRateDistribution = BINOMIAL;
    }
    else if (strcmp(codingRateDistributionString, "normal") == 0)
    {
        codingRateDistribution = NORMAL;
    }
    else
    {
        throw cRuntimeError(this, "Unsupported coding rate distribution: %s. "
                "Supported ones are: \"uniform\", \"binomial\", \"normal\"",
                codingRateDistributionString);
    }

    cModule *oracleModule = satCom->getSubmodule("oracle");
    oracle = check_and_cast<Oracle*>(oracleModule);

    /*
     * A timer tick represents the start of a new communication slot.
     * By default, the timer starts ticking after one communication slot in order
     *  to let at least a few packets arrive at the queues of the ground station.
     */
    timer = new cMessage("timer");
    scheduleAt(simTime() + communicationSlotDuration, timer);
}

void Terminal::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        handleTimer();
    }
    else if (msg->isName("frame"))
    {
        Frame *frame = check_and_cast<Frame*>(msg);
        handleFrame(frame);
    }
    else
    {
        throw cRuntimeError(this, "The terminal cannot handle the received message: %s. "
                "Supported types are: \"timer\" (selfMessage), \"frame\"", msg->getName());
    }
}

void Terminal::handleTimer()
{
    /* Each communication slot begins with the terminals sending their coding rates to the ground station. */

    CODING_RATE codingRate;
    if (codingRateDistribution == UNIFORM)
    {
        codingRate = (CODING_RATE)intuniform(0, 6);
        EV_INFO << "[terminal " << id << "]> Coding rate extracted from a uniform distribution" << endl;
    }
    else if (codingRateDistribution == BINOMIAL)
    {
        /*
         * The probability of success of the binomial distribution is computed so that the mean
         *  coding rate (n*p) of each terminal is sensibly different from all the others.
         */
        double p = (double)(id + 1)/(terminalCount + 1);
        codingRate = (CODING_RATE)binomial(6, p);
        EV_INFO << "[terminal " << id << "]> Coding rate extracted from a binomial distribution with probability of success p: " << p << endl;
    }
    else if (codingRateDistribution == NORMAL)
    {
        double normalCodingRate = normal(3.5, 1);

        if (normalCodingRate < 1)
        {
            codingRate = L3;
        }
        else if (normalCodingRate >= 6)
        {
            codingRate = H3;
        }
        else
        {
            codingRate = (CODING_RATE)normalCodingRate;
        }

        EV_INFO << "[terminal " << id << "]> Coding rate extracted from a normal distribution" << endl;
    }
    else
    {
        throw cRuntimeError(this, "Unsupported coding rate distribution: %d. "
                "Supported ones are: \"uniform\" (0), \"binomial\" (1), \"normal\" (2)",
                codingRateDistribution);
    }

    emit(debugCodingRateDistributionSignal, codingRate);

#ifdef DEBUG_RNGS
    EV_DEBUG << std::fixed << std::setprecision(5) << "[" << simTime().dbl() << "]> Extracted codingRate"
            " for terminal " << id << " | Total RNG calls: " << getRNG(0)->getNumbersDrawn() << endl;
#endif

    CodingRatePacket *codingRatePacket = new CodingRatePacket("codingRatePacket");
    codingRatePacket->setTerminalId(id);
    codingRatePacket->setCodingRate(codingRate);
    sendDirect(codingRatePacket, satellite, "in");

    EV_INFO << "[terminal " << id << "]> For the communication slot [" << simTime().dbl() << " - "
            << (simTime() + communicationSlotDuration).dbl() << "]s my coding rate will be "
            << codingRateToString(codingRate) << endl;

    /* Schedule the next communication slot */
    scheduleAt(simTime() + communicationSlotDuration, timer);
}

void Terminal::handleFrame(Frame *frame)
{
    /* When a frame is received, each terminal asks the oracle where to look (in the frame) to read their messages */
    std::list<PacketLocation> &packetLocations = oracle->getPacketLocations(id);

    if (packetLocations.empty())
    {
        EV_INFO << "[terminal " << id << "]> This frame does not carry any packets for me" << endl;
    }

    /*
     * At the transmitter, packets may be splitted in multiple blocks.
     * To get the original size of each packet, we need to sum the sizes of all the segments.
     */
    int totalPacketSize = 0;

    while (!packetLocations.empty())
    {
        PacketLocation *packetLocation = &packetLocations.front();

        int blockIndex = packetLocation->blockIndex;
        int packetIndex = packetLocation->packetIndex;

        Block *block = &frame->getBlocksForUpdate(blockIndex);
        GenericPacket *packet = block->getPacketsForUpdate(packetIndex);

#ifdef DEBUG_TERMINALS
        EV_DEBUG << "[terminal " << id << "]> Fetched { blockIndex: " << packetLocation->blockIndex
                << ", packetIndex: " << packetLocation->packetIndex << ", isLastSegment: "
                << packetLocation->isLastSegment << "}" << endl;
#endif

        if (packet->getTerminalId() != id)
        {
            throw cRuntimeError(this, "Terminal %d was allowed to read a packet "
                    "destined to terminal %d", id, packet->getTerminalId());
        }

        totalPacketSize += packet->getByteLength();

        /* The delay statistic of a packet can only be emitted when the last segment is fetched */
        if (packetLocation->isLastSegment)
        {
            double delay = (simTime() - packet->getCreationTime()).dbl();
            emit(delaySignal, delay);
            EV_INFO << "[terminal " << id << "]> Received packet { id: " << packet->getTreeId() << ", byteLength: "
                    << totalPacketSize << " } with a delay of " << delay * 1e3 << " ms" << endl;

            totalPacketSize = 0;
        }

        /*
         * Each terminal is responsible for deallocating its own packets.
         * In order to do that, ownership of such packets must be claimed.
         * This is because packets are not sent with a traditional "send()" function,
         *  but carried as objects inside another message (the frame).
         */
        take(packet);
        delete packet;

        packetLocations.pop_front();
    }

    /* Each terminal receives a copy of the same frame, and is responsible for its de-allocation */
    delete frame;
}

void Terminal::finish()
{
    cancelAndDelete(timer);
}
