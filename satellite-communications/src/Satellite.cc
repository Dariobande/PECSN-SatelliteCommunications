
#include "Satellite.h"

Define_Module(Satellite);


void Satellite::initialize()
{
    /*
     * Keeping a pointer to the ground station and to all the terminals allows
     *  the satellite to avoid fetching them each time a message arrives.
     */

    cModule *satCom = getParentModule();
    int terminalCount = satCom->par("terminalCount").intValue();
    terminals.resize(terminalCount);
    int i = 0;
    for (cModule *&terminal : terminals)
    {
        terminal = satCom->getSubmodule("terminal", i++);
    }
    groundStation = satCom->getSubmodule("groundStation");
}

void Satellite::handleMessage(cMessage *msg)
{
    if (msg->isName("codingRatePacket"))
    {
        /* When a codingRate message is received, it must be forwarded to the ground station */

        EV_INFO << "[satellite]> Received a message from terminal " << msg->getSenderModule()->getIndex()
                << ", forwarding it to the ground station" << endl;

        sendDirect(msg, groundStation, "in");
    }
    else if (msg->isName("frame"))
    {
        /* When a frame message is received, it must be forwarded to every and each terminal */

        EV_INFO << "[satellite]> Received a message from the ground station, forwarding it to all the terminals" << endl;

        for (cModule *terminal : terminals)
        {
            /* Duplication is needed because the same cMessage object can't be sent to multiple terminals */
            sendDirect(msg->dup(), terminal, "in");
        }

        /* Since it has been duplicated, the original message can be deleted */
        delete msg;
    }
    else
    {
        throw cRuntimeError(this, "The satellite cannot handle the received message: %s. "
                "Supported types are: \"codingRatePacket\", \"frame\"", msg->getName());
    }
}
