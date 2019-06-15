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
    
Set a PCIe device's link speed to PCI 1.0 or PCI 2.0 (maybe PCI 3.0 I did not try) as my MacPro3,1 does not have PCI 3.0 anyway. I needed this to force my NVMe M2 Carrier card to 5 GT/s as it only negotiates 2.5 GT/s on its own.

    Usage: ./lnkspd bus slot func [target_speed]
    
For instance my card is bus 1 slot 0 func 0 and I want to see the current link speed and max speed:

    sudo ./lnkspd 1 0 0
    
To set speed to PCI 2 I use

    sudo ./lnkspd 1 0 0 2
    
Unfortunately it does not always actually change the link speed. Until then I recommend using pciutils.
    

NOTES
===

* Reading the SMM region will cause the kernel to panic.
* Reading the PCI BAR regions byte at a time with `memcpy()` or `write()`
will will generate all 0xFF since the byte-wise access is not defined.
`rdmem` and `rdpci` will do the right thing with their copy routine.

