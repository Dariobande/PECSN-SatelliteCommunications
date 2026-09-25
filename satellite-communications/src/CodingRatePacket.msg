
import GenericPacket;

enum CODING_RATE 
{
    L3 = 0;
    L2 = 1;
    L1 = 2;
    R  = 3;
    H1 = 4;
    H2 = 5;
    H3 = 6;
};

cplusplus
{{
static const char *codingRateToString(CODING_RATE codingRate)
{
	switch (codingRate)
    {
        case L3:
            return "L3";
        case L2:
            return "L2";
        case L1:
            return "L1";
        case R:
            return "R";
        case H1:
            return "H1";
        case H2:
            return "H2";
        case H3:
            return "H3";
        default:
            throw omnetpp::cRuntimeError("The specified coding rate (%d) does not exist", codingRate);
    }
}

static int codingRateToMaxFrameBytes(CODING_RATE codingRate)
{
	switch (codingRate)
    {
        case L3:
            return 904;
        case L2:
            return 1356;
        case L1:
            return 1808;
        case R:
            return 2260;
        case H1:
            return 2712;
        case H2:
            return 3164;
        case H3:
            return 3616;
        default:
            throw omnetpp::cRuntimeError("The specified coding rate (%d) does not exist", codingRate);
    }
}

static int getMaxTheoreticalFrameBytes()
{
    return codingRateToMaxFrameBytes(H3);
}
}};

packet CodingRatePacket extends GenericPacket  
{
    CODING_RATE codingRate;
};
