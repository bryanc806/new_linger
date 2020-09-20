This is a project to replace the firmware on a Linger Enterprises 65/9028 video terminal from the mid-80s.

See the article in MicroCornucopia, and various adds in electronics magazines at the time:

https://usermanual.wiki/Document/MicroCornucopia2323Apr85.575398179/view

https://worldradiohistory.com/hd2/IDX-Consumer/Archive-Radio-Electronics-IDX/IDX/80s/1986/Radio-Electronics-October-1986-OCR-Page-0112.pdf

Basically, it'a 6502 single board computer with a CRT9028 or CRT9128 display driver to drive a composite monochrome monitor (or 15.75kHz TTL monitor)

I've had this since the '80s, but recently pulled it back out to tinker with.  I ended up rewriting the firmware using cc65 and trying to make it more modern to support better vt100 emulation and faster display.

The original could easily keep up with it's maximum baud rate of 19200 baud without having to use flow control (~1920 characters per second); however, by modifying the board slightly and simplifying the firmware (E.g. removing printer support, extra emulations etc), and bumping the CPU clock rate, I was able to get it to easily work at 8000 characters per second at 115200 baud.  This obviously requires flow control, but makes the terminal much more usable.

A video of this in action along with my explainations is at https://youtu.be/DRY504ln2-A

Dependencies:

You will need cc65.

Building:

Building is done with build.sh.  The sbc.cfg file defines the various memory regions.  I use a 256kbit eeprom, so I end up concatenating the a.out file with itself to make the final image to burn to the eeprom.

TBD:

The vt100 emulation is not complete; it works for top and MicroEmacs, but not for vi.  I suspect this is to do with inserting and deleting lines and or scrolling regions.


