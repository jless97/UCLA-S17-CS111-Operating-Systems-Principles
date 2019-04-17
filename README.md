# CS 111 - Operating Systems Principles, Spring 2017 

## Course Objective
* Provide all students with OS related concepts and exploitation skills that they (a) will all likely need and (b) are unlikely to get in other courses.
* Provide Computer Science majors with an introduction to key concepts and principles that have emerged from (or been best articulated in) operating systems.
* Provide all students with a conceptual foundation that will enable them to read well written introductory level OS-related papers, and engage in intelligent discussions of those topics and participate in entry-level OS-related projects.

## Course Description
Introduction to operating systems design and evaluation. Computer software systems performance, robustness, and functionality. Kernel structure, bootstrapping, input/output (I/O) devices and interrupts. Processes and threads; address spaces, memory management, and virtual memory. Scheduling, synchronization. File systems: layout, performance, robustness. Distributed systems: networking, remote procedure call (RPC), asynchronous RPC, distributed file systems, transactions. Protection and security. Exercises involving applications using, and internals of, real-world operating systems.

### Assignment Topics
* Processes and threads
* File I/O and IPC
* Synchronization and contention
* File Systems
* IoT and embedded system security

### Assignment Overviews
* P0 (C) - Warm-Up
  * Determine whether or not students are prepared to work on C programming projects
* P1A (C) - Terminal I/O and Inter-Process Communication
  * Build a multi-process telnet-like client and server (via full duplex terminal I/O and polled I/O)
* P1B (C) - Encrypted Network Communication
  * Continuation of P1A by passing I/O over TCP socket and adding encrypted communication
* P2A (C) - Races and Synchronization
  * Engage (at a low level) with a range of synchronization problems that deals with conflicting read-modify-write ops on single variables and complex data structures (an ordered linked list)
* P2B (C) - Lock Granularity and Performance
  * Analysis of mutex and spin-lcok as bottlenecks that prevented parallel access to the list. Implementation of new option to divide list into sublists and support synchronization on sublists to allow parallel access to the original list
* P3A (C) - File System Interpretation
  * Understanding the on-disk data format of the EXT2 file system. Write a program to analyze the file system and summarize its contents
* P3B (Python) - File System Consistency Analysis
  * Analyze the summaries of part A to detect errors
* P4A (Intel Edison, C) - Edison Bring-Up
  * Familiarize ourselves with the Intel Edison
* P4B (Intel Edison, Grover Sensor Kit, C) - Edison Sensors
  * Create applications that run in an embedded system, read data from external sensors, and log the results
* P4C (Intel Edison, Grover Sensor Kit, C) - Internet of Things Security
  * Extend the embedded temperature sensor to accept commands from, and send reports back to a network server with encrypted channels