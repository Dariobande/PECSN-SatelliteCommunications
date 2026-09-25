
package satellite_communications;

simple Terminal 
{
	parameters:
	    string codingRateDistribution = default("uniform");
		@signal[delay](type = double);
		@statistic[delayStat](
	   		source = delay;
	   		record = histogram, stats, vector?;
		);
		@signal[debugCodingRateDistribution](type = int);
		@statistic[debugCodingRateDistributionStat](
	   		source = debugCodingRateDistribution;
	   		record = vector?;
		);
		@display("i=device/cellphone2");
	   
	gates:
	    input in @directIn;
}
