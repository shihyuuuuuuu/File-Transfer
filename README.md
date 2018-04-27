# File Transfer using TCP and UDP

Implementing file transmission program on Linux platform.
Including,
* Using TCP to implement file transmission
* Using UDP to implement file transmission

## Environment

Ubuntu 16.04

## Usage
Five arguments need to be passed to the execution file.
From left to right are tcp/udp, sender/receiver, port, host, file_path.

**For tcp**

$ ./lab1_file_transfer tcp send <ip> <port> test_input.txt

$ ./lab1_file_transfer tcp recv <ip> <port>

**For udp**

$ ./lab1_file_transfer udp send <ip> <port> test_input.txt

$ ./lab1_file_transfer udp recv <ip> <port>


## Author
[Shih-Yu](https://github.com/)
