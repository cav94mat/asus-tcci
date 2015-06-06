asus-tcci
======

TrendChip Command Interpreter for ASUS xDSL modem/routers.

**All builds**: [as GitHub Releases](https://github.com/cav94mat/asus-tcci/releases/)

**Updates**: [on GitHub](https://github.com/cav94mat/asus-tcci)

This tool lets you interact with the command interpreter (CI) of your TrendChip-based
device (like the *Asus DSL-N55U*). By sending commands to it, you can access a great number of
low-level options and information related to the chipset itself (and the xDSL modem component in
particular).

You can find exhaustive lists of commands online (just google something like "TrendChip CI Command Reference").

Possibly, this tool can be used also with other chipsets and/or device brands,
since all what it does is establishing a simple communication over (special)
Ethernet frames.

Installation
---
Assuming you already have enabled telnet (you find the related option under _Administration_,
in the router settings web-interface), or even better SSH, you can easily get the binary by issuing these
commands:

1. `[ -d "/opt/sbin" ] && cd /opt/sbin || cd /tmp`
2. `wget --no-check-certificate -O asus-tcci "https://github.com/cav94mat/asus-tcci/releases/download/0.1-150530A/asus-tcci"` (we use **--no-check-certificate** just because our router doesn't come with an up-to-date list of CAs, so it will *always* fail when it's asked to validate HTTPS certificates; however, if you solved this issue on your own device, of course you can omit this flag).
3. `chmod +x ./asus-tcci`

And, finally, `./asus-tcci` to launch it.

Keep in mind, if you don't have OptWare/EntWare (or at least one USB application) installed, you may have to repeat the above steps after each (re)boot, in order to use this utility again. Otherwise, the next time you login via telnet/SSH, just issue `asus-tcci` (you don't need to _cd_ again).

Usage
---
The **asus-tcci** binary support some parameters and operands. It should work just
fine without specifying any of them, however you may still want to alter its
behaviour or make it run on different hardware. The syntax is:

`asus-tcci [-a|--adapter="<adapter1>"] [-b|--remote-adapter="<adapter2>"] [-c|--close] [-p|--log-packets] [-v|--verbose] [-k|--blink-on-receive] [<command>]`

Where **&lt;command&gt;** is the initial command (`sys ver` if none is specified).

The other options are:
* **-a** or **--adapter** to specify the network adapter used to communicate with the CI (default is `eth 2.1`).
* **-b** or **--remote-adapter** to specify the network adapter used to initialize the input socket (default is `eth 2`).
* **-c** or **--close** to quit upon sending the first command (useful for init/scripts).
* **-p** or **--log-packets** to log net packets (sent and received) to STDERR. You can easily redirect STDERR to a file through your shell, if you prefer.
* **-v** or **--verbose** to print debug messages to STDERR (useful to diagnose problems, or when porting the program to other platforms).
* **-k** or **-blink-on-receive** to "blink" when a packet is received.

Alternatives:
* **-V** or **--version** to print asus-tcci version information and quit.
* **-h** or **--help** to print usage information and quit.

Bugs and issues
---
Check the [Issues section](https://github.com/cav94mat/asus-tcci/issues/) for a list of the known (un)solved bugs.

If you managed to correct one, feel free to pm me and/or send a pull-request.
Instead, if you found a new **unreported** bug, please add it.

Building
---
If you want to build this program on your own, or integrate it in a custom firmware, please check the [INSTALL](INSTALL) file.
