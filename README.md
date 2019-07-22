# potato.contracts

## Version : 1.5.1

The design of the POTATO blockchain calls for a number of smart contracts that are run at a privileged permission level in order to support functions such as block producer registration and voting, token staking for CPU and network bandwidth, RAM purchasing, multi-sig, etc.  These smart contracts are referred to as the system, token, msig and wrap (formerly known as sudo) contracts.

This repository contains examples of these privileged contracts that are useful when deploying, managing, and/or using an POTATO blockchain.  They are provided for reference purposes:

   * [pc.system](https://github.com/POTATO-COIN/potato.contracts/tree/master/pc.system)
   * [pc.msig](https://github.com/POTATO-COIN/potato.contracts/tree/master/pc.msig)
   * [pc.wrap](https://github.com/POTATO-COIN/potato.contracts/tree/master/pc.wrap)

The following unprivileged contract(s) are also part of the system.
   * [pc.token](https://github.com/POTATO-COIN/potato.contracts/tree/master/pc.token)

Dependencies:
* [potato v1.4.x](https://github.com/POTATO-COIN/potato/releases/tag/v1.4.4)
* [potato.cdt v1.4.x](https://github.com/POTATO-COIN/potato.cdt/releases/tag/v1.4.1)

To build the contracts and the unit tests:
* First, ensure that your __potato__ is compiled to the core symbol for the POTATO blockchain that intend to deploy to.
* Second, make sure that you have ```sudo make install```ed __potato__.
* Then just run the ```build.sh``` in the top directory to build all the contracts and the unit tests for these contracts.

After build:
* The unit tests executable is placed in the _build/tests_ and is named __unit_test__.
* The contracts are built into a _bin/\<contract name\>_ folder in their respective directories.
* Finally, simply use __clpc__ to _set contract_ by pointing to the previously mentioned directory.
