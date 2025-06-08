# M151M Project #3 : Out-of-order Processor

**Description**:

In this project, you will implement a out-of-order RISC-V processor using C++.

The ISA of this processor is the standard RV32-I base extension.

The provided source code contains an incomplete implementation of the CPU.
The processor implementation consists of the following parts:
* common:       shared directory of utility libraries
* src:          main source directory
* tests:        tests directory
* Makefile:     build make file
* README.md:    this document

You are responsible for completing the code of the out-of-order pipeline such that you can successfully execute the provided tests with the correct timing.
This codebase contains the partial implementation of the following modules:

RAT.h:     Register Alias Table
RST.h:     Register Status Table
RS.h/cpp:  Reservation Stations
ROB.h/cpp: Reorder Buffer
FU.h/cpp:  Functional Units
ooo.cpp:   Out of order logic 

Refer to **TODO** entries inside individual files for where you should make your changes.

## Testing your code
use command line option (-s) to enable display of CPU performance stats.

The provided Makefile contains a `test` command to execute all provided tests.

    $ make test

All tests are under the /tests/ folder.
You can execute an individual test by running:

    $ ./tinyrv -s tests/rv32ui-p-sub.hex

If a test succeeds, you will get "PASSED!" output message.

## Debugging your code
You need to build the project with DEBUG=```LEVEL``` where level varies from 0 to 5.
That will turn on the debug trace inside the code and show you what the processor is doing and some of its internal states.

    $ DEBUG=3 make
    $ ./tinyrv -s tests/rv32ui-p-sub.hex

## What to submit
**A zip file of your source code.
When done with your changes, execute ```make submit``` to generate the submission.zip.
Do not use another method for creating the zip file.

    $ make submit

Please submit the **submission.zip** file.

## Build instructions
* run ```make``` at the root of the project to build the simulator

    $ make

* A linux development environment is needed to build the project
* We recommend using a Ubuntu 18.04 or above distribution
* This C++ project requires C++ 11 to compile and it should come installed on Ubuntu 18.04 or above

## Guidelines
* Only modify the files with the **TODO** entries we will replace the other files during evaluation.
* Do not remove an existing file from the project.
* Do not change the Makefile, you can always add a new file as a header. Make sure to include the new file under the **src/** folder.

## Grading
* simulator compiles successfully: 1 pts
* processor execute successfully: 10 pts.
* processor execute successfully with correct timing: 4 pts.

Based on the coverage of test suites, you will get partial scores.

***Please do not procrastinate.***

## Resources
* RISC-V ISA Specs: https://riscv.org/wp-content/uploads/2017/05/riscv-spec-v2.2.pdf
* Class materials.
