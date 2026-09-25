
import CodingRatePacket;

class Block extends cObject
{
    int maxBytes;
    int usedBytes;
 	CODING_RATE codingRate;
    GenericPacket *packets[];
};

packet Frame
{
    Block blocks[];
};
