
package satellite_communications;

module GroundStation
{
    parameters:
        @display("i=device/receiverdish");

    gates:
		input in @directIn;
        
    submodules:
        packetGenerator[parent.terminalCount]: PacketGenerator;
        packetScheduler: PacketScheduler;
        
    connections:
        in --> packetScheduler.groundStationIn;
        for i = 0 .. parent.terminalCount - 1 
        {
	        packetGenerator[i].out --> packetScheduler.generatorsIn++;
        }
}
