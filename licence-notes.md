# Notes on licensing in ViGraph

I have deliberately used the strongest possible licence (GNU Affero GPL v3)
to ensure ViGraph remains Free Software forever.

Without overriding the much more accurate legal text in the AGPL, if
you are using ViGraph for your own projects, even commercial ones,
and even if you add to it, you'll be fine.  If you want to distribute your
changed version to anyone else (for free or paid) then you need to publish
your changes - ideally, pass them back to me so I can add them for others to
use as well!

If you want to offer a commercial software product or service based on
ViGraph, it's possible under AGPL, but it will give you some
headaches.  Talk to me to see if we can partner commercially on it.

If you want to embed ViGraph in a physical product (say, a laser controller
or mixing desk), again, you can do it under very strict conditions (you would
be required to publish any modules you add, and 
to enable users to build and replace the firmware).  But it
would probably be much more effective to talk to me about a commercial
licence and support package.

Paul Clark

[paul@sandtreader.com](mailto:paul@sandtreader.com)

## External libraries

We have to be careful about the licences of external libraries used in ViGraph
modules for [compatibility](https://www.gnu.org/licenses/license-list.html#GPLCompatibleLicenses) with GPL/AGPL.

(please check licence compatibility and expand this section for each
external library used in modules)

### SDL

Used in

* `modules/audio/sdl-{in,out}`
* `modules/audio/wav-in`
* `modules/bitmap/sdl-out`
* `modules/bitmap/image-in`

[SDL2](https://www.libsdl.org/) uses the [Zlib licence](https://www.libsdl.org/license.php) which is GPL-compatible.

### ASound

Used in

* `modules/audio/alsa-{in,out}`
* `modules/midi/alsa-{in,out}`

The [ALSA Project](https://alsa-project.org/) [library](https://github.com/alsa-project/alsa-lib) is under LGPL-2.1, which is GPL-compatible.

### OLA

Used in `modules/dmx/ola-{in,out}`

The [Open Lighting Architecture](https://www.openlighting.org/ola/) [library](https://github.com/OpenLightingProject/ola) is under LGPL-2.1 (or later),
which is GPL-compatible.

### SoundTouch

Used in `modules/audio/pitch-shift`

The [SoundTouch](http://soundtouch.surina.net/) [library](https://gitlab.com/soundtouch/soundtouch) is LGPL2.1, which is GPL compatible.

### Mosquitto

Used in `modules/iot/mqtt-{in,out}`

The [Eclipse Mosquitto](https://github.com/eclipse/mosquitto) project has the following [licence](https://github.com/eclipse/mosquitto/blob/master/LICENSE.txt):

    This project is dual licensed under the Eclipse Public License 1.0 and the
    Eclipse Distribution License 1.0 as described in the epl-v10 and edl-v10 files

Making use of the EDL option (which is basically new-BSD), allows us to link
with our AGPLv3 code.

### OpenSSL

Used in `modules/time-series/web-fetch` for HTTPS support.

The [OpenSSL](https://openssl.org) library has [two possible licences](https://www.openssl.org/source/license.html):

1. Before 3.0.0, a dual OpenSSL and SSLeay licence, with advertising
requirements, which is [incompatible with GPL](https://www.gnu.org/licenses/license-list.html#OpenSSL).

2. From 3.0.0, the Apache Licence v2.0, which is [GPLv3-compatible](https://www.gnu.org/licenses/license-list.html#apache2).

The current Linux build makes use of the standard libssl-dev package which
is currently 1.1.1.  However, we make use of the 'part of the operating system'
[exemption](https://www.openssl.org/docs/faq.html#LEGAL2),
which is fine unless ViGraph ever becomes
[part of the main OS]( https://lists.debian.org/debian-legal/2002/10/msg00113.html)

For Windows, it is trickier, because we need to distribute the OpenSSL DLL.  An
update to OpenSSL 3.0.0 is in progress which will solve this.

But in any case, although I don't want to pollute things with an explicit
licence exemption which will become moot anyway, as the primary
copyright owner I am entirely happy for anyone to link the ViGraph
code with OpenSSL, and as far as I know there is no issue in the other
direction.

### QT

Used in `desktop` for Windows.

[QT](https://www.qt.io/) is used only to provide the menu and basic 'about' window in the Windows desktop version.  It is [dual-licenced](https://www.qt.io/licensing/) with LGPLv3/GPLv3 for Open Source use, which is obviously fine.  If you want to distribute ViGraph as a
closed-source product under a commercial licence from me, you will also need to obtain a QT commercial licence.

### Windows System Libraries

Used in Windows build for

* `modules/midi/winmm-{in,out}` (winmm)
* `obtools/libs/net` (wsock32, iphlpapi)
* `desktop` (ole32, comctl32, oleaut32, uuid)

These wrapper libraries are provided as part of [MinGW](http://mingw.org),
under a [MIT-style licence](http://mingw.org/license), which is GPL-compatible.

The underlying DLLs are of course installed as part of Windows and need not,
and should not, be distributed.

