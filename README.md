asus-tcci
======

TrendChip Command Interpreter for ASUS xDSL modem/routers.

**All builds**: [as GitHub Releases](https://github.com/cav94mat/asus-tcci/releases/)

**Updates**: [on GitHub](https://github.com/cav94mat/asus-tcci)

This tool lets you interact with the command interpreter (CI) of your TrendChip-based
device (like the *Asus DSL-N55U*). By sending it commands, you can access a great number of
low-level options and information related to the chipset itself (and the xDSL modem component in
particular).

You can find exhaustive lists of commands online ([for instance here](http://www.scribd.com/doc/219586128/TrendChip-CI-Command-Reference-Manual-v1-4-pdf#)).

Possibly, this tool can be used also with other chipsets and/or device brands,
since all what it does is establishing a simple communication over (special)
Ethernet frames.

Installation
---
Provided you have telnet enabled (you find the related option under _Administration_,
in the router settings web-interface), you can easily get the binary by issuing these
commands:

1. `cd /tmp` (if you have a USB drive attached and an existing Optware/Entware setup there, you'd better use `cd /opt/sbin` instead).
2. `wget -o asus-tcci https://github.com/cav94mat/asus-tcci/releases/download/latest/asus-tcci`
3. `chmod +x ./asus-tcci`

And finally, `./asus-tcci` to launch it.

Usage
---
The **asus-tcci** binary support some parameters and operands. It should work just
fine without specifying any option, however you may still want to alter its
behaviour or make it run on different hardware. The syntax is:

`./prova [-a|--adapter="<adapter1>"] [-b|--remote-adapter="<adapter2>"] [-c|--close] [-p|--log-packets] [-v|--verbose] [-k|--blink-on-receive] [<command>]`

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

Sometimes, you might get messed-up or incomplete responses. This probably happens
because other programs are using the CI interface, so you get the responses addressed
to them, or the response you're awaiting for is fetched by the other program.
To prevent this, you may try to suspend offending processes issuing `killall -SIGSTOP <processName>`
via telnet or SSH.

If you're a developer and you found a way to isolate the asus-tcci comm. channel, or
you managed to fix another bug, feel free to pm me or send a pull-request.

If you found a ***new*** bug, instead, please report it into the issues section.

Building
---
If you want to build this program on your own, or integrate it in a custom firmware, please check the [INSTALL](INSTALL) file.
