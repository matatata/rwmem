![Dangerous Software!](https://upload.wikimedia.org/wikipedia/commons/thumb/0/09/Operation_Crossroads_Baker_Edit.jpg/640px-Operation_Crossroads_Baker_Edit.jpg)
Read and Write physical memory on OS X
===

*This ~~can~~ _WILL_ crash your machine!*

- [X] No safety checks
- [X] No validation of content
- [X] No restrictions on where things are written (other than SMM, etc)
- [X] No warranty

The purpose of this tool is to read and write physical memory addresses
of the running system.  It is possible to crash the machine by writing
to arbitrary pages, corrupt the kernel, mess up memory mappings, etc.
It is not recommended for novice users. This is probably not the
chainsaw/sledgehammer/atomic-bomb that you're looking for.

Loading the `DirectHW.kext` gives any root process the ability to
poke anywhere on the system.  It is basically a deliberate backdoor
in the kernel.  You can download it from Snare's site, if you trust him
more than the one bundled in this repository:
http://ho.ax/downloads/DirectHW.dmg

Usage
===

After installing the `DirectHW.dmg` file, load the kernel extension
as root:

    sudo kextutil /System/Library/Extensions/DirectHW.kext

Read your machine's serial number:

    sudo ./rdmem 0xffffff00 256 | xxd -g 1

Read the "BIOS Region" of your boot ROM for analysis (the flash descriptor,
Intel management engine and gig-e sections show up as all 0xFF):

    sudo ./rdmem 0xff990000 0x670000 > mac-bios.bin

NOTES
===

* Reading the SMM region will cause the kernel to panic.
* Reading the PCI BAR regions byte at a time with `memcpy()` or `write()`
will will generate all 0xFF since the byte-wise access is not defined.
`rdmem` and `rdpci` will do the right thing with their copy routine.

PCIe link speed (https://github.com/matatata's addition)
===

    Usage: ./lnkspd bus slot func [target_speed]

I have a PCIe NVMe M2 in my MacPro3,1 PCIe slot 2 (second from below):

    01:00.0 Non-Volatile memory controller: Samsung Electronics Co Ltd NVMe SSD Controller SM981/PM981 (prog-if 02 [NVM Express])

But it only negotiates 2.5GT/s link speed. I only get around 700MB/s R/W in BlackMagic Disk Speed App. I can read the link speed with

    sudo ./lnkspd 01 00 00
    
it echoes

    Vendor 144d Device a808 link speed 1 (max 3) x4 (max x4)
    
Now setting the link speed to 2 does not have any affect when tried on that very device. I don't know why, but setting it to the following device has the desired effect:

    00:01.0 PCI bridge: Intel Corporation 5400 Chipset PCI Express Port 1 (rev 20)

That's what people have also been successfully doing using pcitools, but it took me a while to realize that. In my case 

    Usage: sudo ./lnkspd 00 01 00 2
    
does it! I now have 1400MB/s read write speed!
