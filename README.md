
An attempt to add auxiliary input to the Subaru P120 car stereo (i.e., receiver, head unit).  This device, manufactured by Panasonic, uses a variant of the Alpine MBus protocol.  The protocol in this radio is slightly different (how different I don't know yet, but the timing is different) than the solutions found by Olstyle and kjanesch (which this project will borrow heavily from).


From the pseudochanger readme...

    "Alpine MBus is a bidirectional one-wire serial interface used for communication between audio head units and peripherals like CD players, CD changers, and tape decks. A period for each bit is 3 ms, and low/high bits are indicated by how long the line is held low. Low for the first 600us indicates a 1, low for 1800 us indicates a 0. Data is a multiple of four bits, and a four-bit check follows the packet, an XOR of all the other 4-bit chunks.
    
    The Panasonic variant uses different ping and ACK codes, sends some data at different times, and does what looks like type detection by sending out a cycle of codes. The included library reflects these differences.
    
    This project was developed specifically for the 2002 Subaru WRX head unit, model CQ-EF7260A, and should work with other Panasonic head units that have the same 16-pin connector on the back, as also found in some Mazdas and Hondas.

My P120 receiver (also 2002) is a AM/FM/WB with cassette.  It has the 16-pin connector.  It is confusing why it doesn't work with the original pseudochanger code by kevinsjanesch.  The zero and one bit timings are not 600us/1800us but on the order of 250us/325us, which is bizarre.  This timing leaves little room for error in distinguishing the bits.  Perhaps my unit is bad, but that's what I get on my logic analyzer.

# Blog

http://tucsontelegraph.blogspot.com/2017/03/adding-auxbt-to-subaru-stereo-through.html

# Credits / References

http://www.hohensohn.info/mbus/

https://github.com/kjanesch/pseudochanger
https://sites.google.com/site/kevinsjanesch/projects/subaru-line-in

https://github.com/Olstyle/MBus
