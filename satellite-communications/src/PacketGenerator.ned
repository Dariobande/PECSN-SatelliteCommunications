
package satellite_communications;

simple PacketGenerator
{
    parameters:
        int minPacketSize @unit(byte) = default(1 byte);
        int maxPacketSize @unit(byte) = default(500 byte);
        double meanPacketInterarrivalTime @unit(s) = default(40 ms);
       	@signal[debugWithinSlotArrivalTime](type = long);
        @statistic[debugWithinSlotArrivalTimeStat](
	   		source = debugWithinSlotArrivalTime;
	   		record = vector?;
		);
        @display("i=misc/globe");
        
    gates:
        output out;
}
