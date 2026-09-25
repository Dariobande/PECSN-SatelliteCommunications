
package satellite_communications.simulations;

import satellite_communications.GroundStation;
import satellite_communications.Terminal;
import satellite_communications.Satellite;
import satellite_communications.Oracle;

network SatCom
{
    parameters:
        int terminalCount = default(3);
        double communicationSlotDuration @unit(s) = default(80 ms);
        
    submodules:
        groundStation: GroundStation 
        {
            @display("p=89,175");
        }
        terminal[terminalCount]: Terminal 
        {
            @display("p=347,175");
        }
        satellite: Satellite 
        {
            @display("p=212,24");
        }
        oracle: Oracle 
        {
            @display("p=347,24");
        }
}

