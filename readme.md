Unofficial NAD Link protocol specification

This is a decription of how to use the NAD Link RCA connectors at the back of most NAD equipment, using a PC parallell port.

The NAD Link uses a slightly modified version of the NEC remote control protocol, where 0V represents pulse, and +5V represents flat.

The NEC Receiver protocol

Derived from sbprojects.com.

1. Preamble

  9000 μs pulse
  4500 μs flat
2. Address

The (8 bit) address is transferred using pulse distance encoding with the least signficant bit going first over the wire. Afterwards, the bitwise negation is sent. Note: This is where NAD diverges slightly from the NEC spec. More on this later.

One-bits are encoded like this:

  560 μs pulse
 1690 μs flat
Zero-bits are encoded like this:

  560 μs pulse
  560 μs flat
Example: Assume the address is 00110101. In this case 00110101 11001010 is transmitted (space added for readability - not a delay). Sending the bitwise negation after the actual address means that there are always 8 ones and 8 zeroes being transmitted, so all addresses take the same amount ot time to tramsit, despite the use of pulse distance encoding.

3. Command

The command is also 8 bits, and is transmitted in the same manner as the address. First directly, then inverted.

4. Command terminator

  560 μs pulse
42020 μs flat
5. Repeat

Send zero or more of this to indicate that the remote control button is being held down.

 9000 μs pulse
 2250 μs flat
  560 μs pulse
98190 μs flat
(Total time: 110ms)

Summary

PREAMBLE
(address >> 0) & 1
(address >> 1) & 1
(address >> 2) & 1
(address >> 3) & 1
(address >> 4) & 1
(address >> 5) & 1
(address >> 6) & 1
(address >> 7) & 1
(~address >> 0) & 1
(~address >> 1) & 1
(~address >> 2) & 1
(~address >> 3) & 1
(~address >> 4) & 1
(~address >> 5) & 1
(~address >> 6) & 1
(~address >> 7) & 1
(data >> 0) & 1
(data >> 1) & 1
(data >> 2) & 1
(data >> 3) & 1
(data >> 4) & 1
(data >> 5) & 1
(data >> 6) & 1
(data >> 7) & 1
(~data >> 0) & 1
(~data >> 1) & 1
(~data >> 2) & 1
(~data >> 3) & 1
(~data >> 4) & 1
(~data >> 5) & 1
(~data >> 6) & 1
(~data >> 7) & 1
COMMAND TERMINATOR
(Total time: 110ms)

Differences in the NAD implementation

NAD does not send exactly the inverted address as the second byte. Instead, typically 1 or 2 bits are not inverted. This is not a problem, since it is included in the .ir files available at the NAD website.

NAD remote control codes

At the time of writing, NAD publishes the codes for its remote controls at [nadelectronics.com]. I have also mirrored the Creston remote controller codes, since I managed to reverse engineer the file format. I also made C program to decode the files, as well as a C program to control a NAD 912 receiver via TCP

By Morten Hustveit <morten full-stop hustveit at gmail full-stop com> May 2007