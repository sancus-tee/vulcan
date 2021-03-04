# VulCAN: Efficient Component Authentication and Software Isolation for Automotive Control Networks
[![Vulcan CI](https://github.com/sancus-tee/vulcan/actions/workflows/ci.yml/badge.svg)](https://github.com/sancus-tee/vulcan/actions/workflows/ci.yml)
[![License: GPL](https://img.shields.io/badge/License-GPL-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This repository contains the source code accompanying our
[paper](https://distrinet.cs.kuleuven.be/software/sancus/publications/acsac17.pdf)
on automotive network security which appears in the 2017 ACSAC conference.

> Jo Van Bulck, Jan Tobias Mühlberg, and Frank Piessens. 2017. VulCAN:
> Efficient Component Authentication and Software Isolation for Automotive
> Control Networks. In Proceedings of the 33th Annual Computer Security
> Applications Conference (ACSAC'17).

## Abstract

Vehicular communication networks have been subject to a growing number of
attacks that put the safety of passengers at risk. This resulted in millions of
vehicles being recalled and lawsuits against car manufacturers. While recent
standardization efforts address security, no practical solutions are
implemented in current cars. 

This paper presents VulCAN, a generic design for efficient vehicle message
authentication, plus software component attestation and isolation using
lightweight trusted computing technology. Specifically, we advance the
state-of-the-art by not only protecting against network attackers, but also
against substantially stronger adversaries capable of arbitrary code execution
on participating electronic control units. We demonstrate the feasibility and
practicality of VulCAN by implementing and evaluating two previously
proposed, industry standard-compliant message authentication protocols on top
of Sancus, an open-source embedded protected module architecture. Our results
are promising, showing that strong, hardware-enforced security guarantees can
be met with a minimal trusted computing base without violating real-time
deadlines under benign conditions.

## Builing and Running

### 1. Install Sancus toolchain

Our VulCAN prototype is based on the open-source
[Sancus](https://distrinet.cs.kuleuven.be/software/sancus/) embedded protected
module architecture. You should thus first install a working Sancus v2.1
toolchain (compiler, support libraries, and hardware design/simulator).

To get started quickly, we offer an Ubuntu 16.04-based Docker container that
contains all dependencies and a ready-to-use Sancus toolchain. We refer to the
[sancus-main](https://github.com/sancus-tee/sancus-main/tree/master/docker)
repository for detailed instructions to build and run the Docker container.
Alternatively, sancus-main also features instructions and a Makefile to
automatically resolve dependencies and build the latest Sancus projects from
source.

**Note.** VulCAN requires Sancus to be configured to provide 128 bits of
security. Use `make SANCUS_SECURITY=128` to override the default 64-bit
security level. Specifically, to get started with a suitable VulCAN Docker
container, proceed as follows:

```bash
sancus-main/docker$ make build SANCUS_SECURITY=128
sancus-main/docker$ make run
...
root@b1961553622c:/sancus# git clone https://github.com/sancus-tee/vulcan.git
root@b1961553622c:/sancus# cd vulcan
```

### 2. Test your Sancus distribution

To verify whether your Sancus distribution was correctly installed, try one of
the examples programs in the
[sancus-examples](https://github.com/sancus-tee/sancus-examples) repository:

```bash
sancus-examples$ make SANCUS_SECURITY=128 hello-world.sim
```

The above command should successfully build the 'hello-world' binary, and start
simulating it in the Icarus Verilog simulator. Because `sancus-sim` simulates
the CPU behavior a the gate-level, however, simulation can take a while (about
4 to 5 minutes on our machines). Plenty of time to get a coffee ;)

<p align="center">
  <img src="https://distrinet.cs.kuleuven.be/software/sancus/sancus-sim_small.png"/>
  <em>Adapted from <a href="https://www.xkcd.com/303/">xkcd</a>.</em>
</p>


### 3. Build the VulCAN software stack

As explained in the paper, we implemented and evaluated two recently published
CAN authentication protocols:
[vatiCAN](https://www.infsec.cs.uni-saarland.de/~nuernberger/getbibtex.php?type=pdf&citation=nuernberger2016vatican&category=publications)
and [LeiA](https://www.cs.bham.ac.uk/~garciaf/publications/leia.pdf). The
`can-auth` directory contains the source code for these "vulcanized" CAN
authentication libraries, which are selected through the `LIBVULCAN`
environment variable when building the application.

To build the round-trip time benchmark application (sect. 5.2), use:

```bash
vulcan$ make clean bench LIBVULCAN=leia # or choose LIBVULCAN=vatican
```

Likewise, to build the extended application scenario (sect. 6), use:

```bash
vulcan$ make clean demo LIBVULCAN=leia # or choose LIBVULCAN=vatican
```

**TCB size and memory footprint.** To reproduce our source lines of code and
binary size measurements (sect. 5.1), try respectively `make sloc` and `make
size` in the "bench" directory.

### 4. Run VulCAN in the simulator

To be able to easily experiment with VulCAN without having to buy real CAN
transceiver hardware and FPGAs, we developed an elementary "ican" interface
implementation that uses local file I/O to emulate an untrusted CAN bus in the
`sancus-sim` simulator.

Proceed as follows to simulate a distributed embedded application that first
performs a successful authenticated round-trip (ping-pong), and subsequently
forces an authentication failure.

```bash
vulcan/bench$ make clean sim LIBVULCAN=leia # or choose LIBVULCAN=vatican
```

The above command starts the `ecu-recv` simulation process in the background,
and continues with the `ecu-send` simulation on the foreground. The foreground
sender process should output progress information from time to time, but the
entire simulation takes a *long* time (about 45 minutes on our machines).

![sim-output](https://distrinet.cs.kuleuven.be/software/vulcan/images/demo-sim.png)

The simulation output of the testbench application can also be viewed on the
[Travis](https://travis-ci.org/sancus-tee/vulcan) continuous integration web
interface.

**MAC computation times.** The simulator emulates CAN transceiver hardware
using local file I/O, and can therefore *not* be used for macrobenchmark timing
experiments, as real-world CAN driver interaction and network delays are of
course not incorporated. Since the simulator simulates the hardware design at
the gate level, however, it can accurately report the number of CPU cycles
needed for non-I/O operations. To reproduce our MAC computation measurements
(sect. 5.2):

```bash
vulcan/bench$ make mac LIBVULCAN=leia # or choose LIBVULCAN=vatican
```

### 5. Run VulCAN on FPGAs and real CAN hardware

To evaluate the VulCAN approach in terms of performance overhead, we
constructed a testbench where we interfaced Sancus-enabled FPGAs with real CAN
transceiver hardware. Our prototype setup features multiple Xess
[XuLA2-LX25](http://www.xess.com/shop/product/xula2-lx25/) Spartan-6 FPGAs,
each synthesized with a Sancus-enabled OpenMSP430
[core](https://github.com/sancus-tee/sancus-core), and each connected to an
off-the-shelf Modtronix [im1CAN](http://modtronix.com/im1can) SPI CAN bus
peripheral module using the popular Microchip
[MCP2515](http://www.microchip.com/wwwproducts/en/en010406) CAN transceiver
chip.

![fpga-setup](https://distrinet.cs.kuleuven.be/software/vulcan/images/demo.jpg)

Use `make load` to upload the application binaries to the different FPGAs over
a UART USB connection. The Makefile in the "bench" directory furthermore
contains an `OPTS` variable that can be adjusted to configure the application
binary for the various scenarios discussed in the performance evaluation (sect.
5.2).

## License

VulCAN is free software, licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0).
