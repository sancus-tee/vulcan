# VulCAN: Efficient Component Authentication and Software Isolation for Automotive Control Networks
[![Build Status](https://travis-ci.org/sancus-pma/vulcan.svg?branch=master)](https://travis-ci.org/sancus-pma/vulcan)
[![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

This repository contains the source code accompanying our
[paper](https://distrinet.cs.kuleuven.be/software/sancus/publications/acsac17.pdf)
on automotive network security which appears in the 2017 ACSAC conference.

> Jo Van Bulck, Jan Tobias Mühlberg, and Frank Piessens. 2017. VulCAN:
> Efficient Component Authentication and Software Isolation for Automotive
> Control Networks. In Proceedings of the 33th Annual Computer Security
> Applications Conference (ACSAC'17).

## Paper Abstract

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
[sancus-main](https://github.com/sancus-pma/sancus-main/tree/master/docker)
repository for detailed instructions to build and run the Docker container.
Alternatively, sancus-main also features instructions and a Makefile to
automatically resolve dependencies and build the latest Sancus projects from
source.

**Note:** VulCAN requires Sancus to be configured to provide 128 bits of
security. Use `make SANCUS_SECURITY=128` to override the default 64-bit
security level.

### 2. Test your Sancus distribution

To verify whether your Sancus distribution was correctly installed, try one of
the examples programs in the
[sancus-examples](https://github.com/sancus-pma/sancus-examples) repository:

```bash
$ cd sancus-examples
$ make SANCUS_SECURITY=128 hello-world.sim
```

The above command should successfully build the 'hello-world' binary, and start
simulating it in the Icarus Verilog simulator. Because `sancus-sim` simulates
the CPU behavior a the gate-level, however, simulation can take a while (about
4 to 5 minutes on our machines). Plenty of time to get a coffee ;)

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
$ make clean bench LIBVULCAN=leia # or choose LIBVULCAN=vatican
```

Likewise, to build the extended application scenario (sect. 6), use:

```bash
$ make clean demo LIBVULCAN=leia # or choose LIBVULCAN=vatican
```

### 4. Run VulCAN in the simulator

todo

### 5. Run VulCAN on FPGAs with real CAN transceivers

todo

## License

VulCAN is free software, licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0).
