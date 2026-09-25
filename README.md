# PECSN - Satellite Communications

[![Language](https://img.shields.io/badge/Language-C%2B%2B14%20%2F%20C%2B%2B17-blue.svg)](https://isocpp.org/)
[![Simulator](https://img.shields.io/badge/Simulator-OMNeT%2B%2B%205.x%20%2F%206.x-orange.svg)](https://omnetpp.org/)
[![Modeling](https://img.shields.io/badge/Modeling-NED-brightgreen.svg)](https://doc.omnetpp.org/omnetpp/manual/#sec:ned-lang)

[Technical Report](Documentation.pdf) | [Presentation Slides](Presentation.pdf) | [Project Specifications](Specifications.pdf) | [Source Code](satellite-communications/src/)

This repository contains the design, discrete-event simulation modeling, and performance evaluation of a **Satellite Communications System**, implemented in OMNeT++ and C++. The system models a bent-pipe satellite relaying downlink data from a Ground Station (GS) to a set of terrestrial terminals, managing per-terminal dedicated FIFO queues through a slotted transmission protocol and a greedy **Maximum Coding Rate (MaxCR)** frame scheduling policy.

The project was developed for the **Performance Evaluation of Computer Systems and Networks** (PECSN) course (Master of Science in Computer Engineering, **Università di Pisa**), Academic Year 2024–2025.

Authors:
- **Dario Bandecchi**
- **Francesco De Lucchini**
- **Niccolò Mulè**

---

## Project Overview

In satellite communication links, terminal channel conditions vary substantially over time due to atmospheric attenuation, rain fading, and geographical diversity. To maximize overall system efficiency, modern satellite systems employ **Adaptive Coding and Modulation (ACM)** combined with opportunistic frame scheduling.

This simulator studies the trade-offs between system throughput, packet delivery delay, and frame utilization under varying network loads, frame block partitions, and channel coding rate distributions.

Key capabilities of the system include:
- **Slotted Frame Architecture**: Transmissions operate on synchronized communication slots of constant duration (80 ms), matching standard satellite air-interface designs.
- **Dedicated Queueing & Realistic Traffic Generation**: Independent FIFO queues at the Ground Station per terminal, with exponentially distributed inter-arrival times (Poisson traffic arrivals) and uniformly distributed packet lengths.
- **Opportunistic MaxCR Scheduling**: Dynamic frame construction sorting active terminals descending by their reported channel coding rate (from H3 down to L3) to prioritize higher-rate channels while filling frames greedily.
- **Multi-Block Frame Aggregation**: Configurable division of the downlink frame into multiple blocks, enabling flexible traffic multiplexing and terminal packing across heterogeneous channel qualities.
- **Diverse Channel Quality Modeling**: Support for multiple coding rate statistical models (Uniform, Discretized Normal, and heterogeneous Binomial distributions assigning distinct average channel conditions to each terminal).
- **Zero-Overhead End-to-End Tracking**: An out-of-band `Oracle` module that indexes scheduled packet locations and tracks segmentation delays without perturbing network traffic or incurring artificial overhead.
- **Statistically Rigorous Evaluation**: Automated evaluation pipeline supporting warm-up period calibration (Welch method), independent replications (30 runs), isolated RNG streams, and 99% confidence interval calculations.

---

## Technical Specifications

| System Component | Technology / Mechanism | Specification / Details |
| :--- | :---: | :--- |
| **Simulation Engine** | OMNeT++ (5.x / 6.x) | Discrete-event simulation framework written in C++ |
| **Network Topology** | Star / Relay Network | Ground Station, Bent-Pipe Satellite, N Terminals, and Oracle helper |
| **Slot Duration** | Synchronized Slotted Aloha / TDMA | Fixed slot duration of 80 ms |
| **Traffic Model** | M/U/1 per Terminal Queue | Exponential inter-arrivals ($\lambda^{-1} = 40$ ms), Uniform packet size (20–80 bytes) |
| **Coding Rate Levels** | 7 Discrete ACM States | Low rates (`L3`, `L2`, `L1`), Reference (`R`), High rates (`H1`, `H2`, `H3`) |
| **Block Capacities** | Variable Slot Capacity | $904/M$ (`L3`) to $3616/M$ (`H3`) bytes per block in an $M$-block frame |
| **Scheduling Discipline** | MaxCR Policy | Greedy high-to-low coding rate priority with non-preemptive queue serving |
| **Channel Distributions** | Stochastic Ensembles | Uniform ($U(0, 7)$), Normal ($\mu=3.5, \sigma=1$), and Terminal-dependent Binomial |
| **RNG Stream Isolation** | Dual-Stream per Terminal | $2 \times N$ RNGs ($2i$ for packet arrival/size, $2i+1$ for terminal coding rate) |
| **Statistical Methodology** | Independent Replications | 30 replications per configuration, 10 s warm-up removal, 800 s run limit |

---

## System Architecture & Simulation Model

### 1. Network Modules

The OMNeT++ network (`SatCom`) is organized hierarchically into the following modules:

- **`GroundStation`**: Compound module responsible for downlink traffic generation and frame composition.
  - **`PacketGenerator[N]`**: Array of $N$ traffic sources (one per terminal), generating packets of size $S \sim \mathcal{U}(\text{minSize}, \text{maxSize})$ with inter-arrival time $T \sim \text{Exp}(\mu_T)$.
  - **`PacketScheduler`**: Maintains dedicated FIFO queues for each terminal, receives uplink coding rate reports, runs the MaxCR algorithm to populate $M$ blocks per frame, and emits transmission statistics.
- **`Satellite`**: Acts as a transparent bent-pipe relay. In the uplink direction, it forwards terminal coding rate notifications to the Ground Station; in the downlink direction, it broadcasts assembled frames to all active terminals.
- **`Terminal[N]`**: Array of $N$ independent receiving ground stations. At the start of each slot, each terminal samples a random channel state from its configured distribution, signals its coding rate, filters received frames for packets matching its ID, and measures end-to-end packet delays.
- **`Oracle`**: Passive coordination module that stores the exact block and packet indexing of transmitted traffic, enabling terminals to accurately collect delay metrics when whole packets are received.

### 2. Slotted Transmission Protocol & MaxCR Policy

Each 80 ms communication slot follows a strict three-phase cycle:

1. **Uplink Channel Reporting**: Each terminal $i$ draws a Coding Rate ($CR_i$) from its configured probability distribution and transmits a `CodingRatePacket` to the Ground Station via the satellite.
2. **Frame Scheduling & Composition**:
   - The `PacketScheduler` sorts all terminals in descending order of reported coding rate ($H3 > H2 > H1 > R > L1 > L2 > L3$).
   - For each frame block $j \in \{0, \dots, M-1\}$, the block's coding rate is set by the first terminal scheduled within that block.
   - The block's data capacity in bytes is determined according to the ACM specification table:
     $$\text{Capacity}(CR, M) = \frac{\text{BaseCapacity}(CR)}{M}$$
   - The scheduler greedily drains the terminal's queue. If remaining capacity permits, packets from subsequent terminals are packed into the same block, provided their reported coding rate is greater than or equal to the block's established rate.
   - Any packet that cannot fit entirely within the remaining block capacity is held in queue for subsequent slots (no packet fragmentation across frames).
3. **Downlink Broadcasting & Reception**: The completed `Frame` is broadcast through the satellite to all terminals, which extract their respective packets and record performance metrics.

### 3. Coding Rates & Capacity Scale

| Coding Rate (CR) | Base Slot Bytes ($M=1$) | Block Capacity ($M$ Blocks) | Theoretical Peak Bitrate |
| :---: | :---: | :---: | :---: |
| **L3** | 904 B | $904 / M$ B | 90.4 kbps |
| **L2** | 1356 B | $1356 / M$ B | 135.6 kbps |
| **L1** | 1808 B | $1808 / M$ B | 180.8 kbps |
| **R** | 2260 B | $2260 / M$ B | 226.0 kbps |
| **H1** | 2712 B | $2712 / M$ B | 271.2 kbps |
| **H2** | 3164 B | $3164 / M$ B | 316.4 kbps |
| **H3** | 3616 B | $3616 / M$ B | 361.6 kbps |

### 4. Experimental Factors & Channel Scenarios

The simulator evaluates the system across three primary experimental factors:
- **Terminal Population ($N$)**: Evaluated for $N \in \{10, 20, 25, 30, 35, 50\}$, capturing under-loaded, balanced, and heavily congested regimes.
- **Blocks per Frame ($M$)**: Dynamically scaled relative to terminal count ($M = 0.2N, 0.6N, 1.0N, 1.4N, 1.8N$) to assess multiplexing granularity.
- **Channel Quality Distributions**:
  - **Uniform**: $CR \sim \mathcal{U}\{L3, \dots, H3\}$ with equal probability across all states.
  - **Discretized Normal**: $CR \sim \mathcal{N}(\mu=3.5, \sigma=1)$ centered around the reference rate $R$.
  - **Binomial**: Terminal-specific channel quality with $n=6$ and $p_i = \frac{i+1}{N+1}$, modeling an environment where different terminals experience systematically different channel conditions.

---

## Key Performance Indicators

- **Throughput ($S$)**: Total payload bits successfully delivered by the Ground Station divided by simulation time (measured in kbps).
- **Mean Packet Delay ($D$)**: Average latency elapsed between packet creation at the `PacketGenerator` and complete delivery at the receiving `Terminal`.
- **Mean Frame Utilization ($U$)**: Average ratio of scheduled payload bytes relative to the total byte capacity available in assembled frames.

---

## Project Structure

```text
PECSN-SatelliteCommunications/
├── Documentation.pdf               # Comprehensive Technical Report (31 pages)
├── Presentation.pdf                # Project Presentation Slides (11 slides)
├── Specifications.pdf              # Initial Project Assignment Specifications
├── README.md                       # Repository Architecture and Usage Documentation
└── satellite-communications/       # OMNeT++ Simulation Root
    ├── Makefile                    # Root compilation script
    ├── simulations/
    │   ├── SatCom.ned              # Top-level OMNeT++ network topology
    │   ├── omnetpp.ini             # Simulation profiles (General, Debug, WarmUp, DataAnalysis)
    │   ├── package.ned             # Simulation package definition
    │   └── run                     # Shell execution script
    └── src/
        ├── GenericPacket.msg       # Base packet definition with destination terminalId
        ├── CodingRatePacket.msg    # Terminal channel state signaling message
        ├── Frame.msg               # Downlink transmission frame container
        ├── GroundStation.ned       # Compound ground station module definition
        ├── PacketGenerator.cc/.h/.ned # Traffic generator (exponential arrival, uniform size)
        ├── PacketScheduler.cc/.h/.ned # FIFO queueing and MaxCR scheduling engine
        ├── Satellite.cc/.h/.ned    # Bent-pipe relay node implementation
        ├── Terminal.cc/.h/.ned     # Receiver terminal and statistics sink
        ├── Oracle.cc/.h/.ned       # Metadata coordinator for end-to-end packet tracking
        ├── makefrag                # Build configuration and debugging directives
        └── package.ned             # OMNeT++ source namespace package
```

---

## Getting Started

### Prerequisites

- **OMNeT++**: Version 5.6+ or 6.0+ (including the graphical simulation IDE).

### Running the Simulator

1. **Import Project**:
   Open the OMNeT++ IDE and import the `satellite-communications` project into your workspace (`File` > `Open Projects from File System...` or `Import...` > `General` > `Existing Projects into Workspace`).

2. **Build Project**:
   Right-click on the `satellite-communications` project in the Project Explorer and select **Build Project** (or use the shortcut `Ctrl+B` / `Cmd+B`). The IDE automatically builds the simulation model and compiles the C++ sources.

3. **Launch Simulations**:
   In the Project Explorer, open `satellite-communications/simulations` and locate `omnetpp.ini`. Right-click on `omnetpp.ini` and choose **Run As** > **OMNeT++ Simulation**.

   From the simulation launcher dialog, you can select the desired execution profile:
   - **`Debug`**: Runs an interactive visual simulation in the graphical environment (**Qtenv**) with real-time packet exchange animations, queue monitoring, and event logs.
   - **`WarmUpTimeCalibration`**: Runs the calibration runs designed to evaluate the transient phase and determine the steady-state warm-up period.
   - **`DataAnalysis`**: Runs the complete simulation campaign across all factor combinations ($N$, $M$, and coding rate distributions) with 30 independent repetitions.

All simulation output scalar (`.sca`) and vector (`.vec`) files are automatically generated inside `simulations/results/` and can be inspected directly using OMNeT++'s built-in Analysis Tool.
