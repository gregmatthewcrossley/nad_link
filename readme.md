# Control a NAD receiver with an Arduino
The [NAD C 740 stereo receiver](https://www.hifiengine.com/manual_library/nad/c740.shtml) (and probably many others) have a special RCA port in the back, which they call NADLink. Its there so that different NAD components can share IR commands with one another. 

But there's nothing magic about the protocol! In fact, its just a rehash of the NEC remote control protocol, where instead of IR pulses it uses 5V TTL over a simple two-conductor RCA cable.

Hopefully, the [Arduino code in this repo](/NADLink.ino) is helpful to anyone else who's trying to control a NAD receiver via a microcontroller!

Special thanks to Morten Hustveit and [Bitraf](https://github.com/bitraf) for their research and code sharing.