# ViGraph Server

This is the "server side" of the ViGraph system, containing the core 'engine' and modules.  The engine has little (Windows) or no (Linux) user interface itself - all the user interaction is through a Web application (see [vg-ui-web](https://github.com/vigraph/vg-ui-web)).  It's written in C++14 and is currently ported to Linux (with Debian and RPM packaging) and Windows (through cross-compilation with [MinGW](https://mingw.org).

ViGraph was created in 2017 by [Paul Clark](https://sandtreader.com), initially to create interactive laser displays for his company [Greenwave Interactive](https://greenwaveinteractive.com).  Alex Woods at Paul's other company [Packet Ship](https://www.packetship.com) significantly improved the core engine dataflow system and added the audio and MIDI modules in 2019/20.  Paul published the whole of ViGraph under AGPLv3 in July 2020.

## What's it for?

Briefly, ViGraph is a platform for creating complex systems by plugging together simple modules, either graphically, or in a simple text language (VG).  It started as a primarily creative platform for audio and laser graphics, but it is expanding its horizons all the time...

It currently has modules for:

* Audio synthesis
* Vector graphics, including laser output
* Bitmap graphics, including LED output
* MIDI
* DMX lighting
* IoT and sensor interfaces
* Maths and physics simulation
* Time series data processing and visualisation

You can find out more about ViGraph (with prettier pictures) at [vigraph.com](https://vigraph.com)

## Building ViGraph

The ViGraph server is built on top of Paul's [ObTools](https://github.com/sandtreader/obtools) libraries, and uses its build system.  See the README there for details.  You don't need to check out ObTools separately, it's a submodule of `vg-server`, so after cloning this repository you just need to:

        $ git submodule init
        $ git submodule update
        
The complete build process is therefore:

        $ git clone git@github.com:vigraph/vg-server.git
        $ cd vg-server
        $ git submodule init
        $ git submodule update
        $ obtools/build/init.sh -t release               -- or -t debug if you are developing
        $ tup
        
### Dependencies

Building the Linux version requires a standard build system with `clang++`, and `tup`.  The latest Debian (10) & Ubuntu (20.04) have Tup packaged, otherwise you can build it yourself from the [Tup sources](http://gittup.org/tup/).

To build all the modules, you will require:

* SDL2 (libsdl2-dev)
* ALSA (libasound2-dev)
* OLA (libola-dev)
* plus all the [dependencies to build ObTools](https://github.com/sandtreader/obtools/blob/master/README.md#dependencies)

## Contributions

Yes please!

If it's a bug-fix, test or tidy, please just go ahead and send a PR.  If it's anything major, please discuss it with me first...

I ask all contributors to sign a standard, FSF-approved [Contributor License Agreement](http://contributoragreements.org/) to make the project easier to manage.  You can sign it when you generate a PR, or in advance [here](https://cla-assistant.io/vigraph/vg-server).  You only have to do this once for all of ViGraph and ObTools.

Thanks!

        
