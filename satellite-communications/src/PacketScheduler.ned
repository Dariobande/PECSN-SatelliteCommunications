
package satellite_communications;

simple PacketScheduler
{
    parameters:
        double blocksPerFrame = default(4);
        @signal[throughput](type=double);
        @statistic[throughputStat]
        (
        	source = throughput;
        	record = last, vector?;
        );
        @signal[frameUtilization](type=double);
        @statistic[frameUtilizationStat]
        (
        	source = frameUtilization;
        	record = histogram, stats, vector?;
        );
        @signal[debugInstantaneousThroughput](type=double);
        @statistic[debugInstantaneousThroughputStat]
        (
        	source = debugInstantaneousThroughput;
        	record = histogram, stats, vector?;
        );
        @signal[debugFrameSize](type=long);
        @statistic[debugFrameSizeStat]
        (
        	source = debugFrameSize;
        	record = vector?;
        );
        @display("i=block/join");
        
    gates:
        input generatorsIn[];
        input groundStationIn;
}
