# Pico Explorer Infrared Reader

This project configures a
[Pico Explorer Base](https://shop.pimoroni.com/products/pico-explorer-base) to
respond to IR remote signals. The "data" byte from the signal is displayed on
screen, so that you can quickly document the codes used by a new remote. It also
supports a few of the basic features I used in my lighting projects, such as
changing the brightness and colour of the lights, and selecting various
colour-cycling modes.

## Prerequisites

### Hardware

First, you need the right hardware, starting with a Raspberry Pi Pico and a Pico
Explorer Base. It should not matter whether you have a Pico that supports
wireless or not.

You will also need an IR receiver (I used
[this one](https://www.tinytronics.nl/en/communication-and-signals/wireless/infrared/ir-infrared-receiver-module-38khz-940nm)).
Connect the `ground` and `vcc` pins to the `ground` and `3v3` headers on the
explorer base.  Connect the `out` pin to the header for `GP5` (you can use
another pin, but you'll need to update the code). 

Finally, you'll need a remote.  I have tested this project with:

1. [The remote from this
   kit](https://www.tinytronics.nl/en/communication-and-signals/wireless/infrared/ir-sensor-module-with-remote-and-battery-with-ir-led)
2. [The remote for the party bulbs I
   have](https://www.amazon.nl/-/en/Changing-Dimmable-Control-Colours-Decoration/dp/B06XYFZ4J5/ref=sr_1_1?sr=8-1).
3. The remote for [the LED curtains I
   use](https://nl.aliexpress.com/item/1005010344044910.html).

### Software

You also need a working development environment. First, you need to set up the
basics, i.e. the tools to compile code and the libraries you need. I followed
the steps in the
[setup guide for the Pico](https://github.com/pimoroni/pimoroni-pico/blob/main/setting-up-the-pico-sdk.md).

If you want to work with VS Code, you'll also need to go through something like
[this guide from Digikey](https://www.digikey.be/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-2-debugging-with-vs-code/470abc7efb07432b82c95f6f67f184c0),
which helpfully covers the non-obvious bits required to get openocd working.

If you're interested, the [first part in the DigiKey series](https://www.digikey.be/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-1-blink-and-vs-code/7102fb8bca95452e9df6150f39ae8422)
provides a nice alternative to the Pico guide.

## Building and Installing

### VS Code and a PiProbe

If you have a [PiProbe](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html), 
you should be able to use the debugger configuration in this project to build,
install (and debug) the code in this project.

First, you should check the paths in `.vscode/launch.json`and
`.vscode/settings.json` and update them as needed to match your system. Then,
you should be able to just hit the debugger icon and choose the configuration
defined in `.vscode/launch.json`.

The application will be built, deployed, and will pause execution at the
beginning of the `main()` function.

### Command Line Tools

To build the application from the command line, you can use commands like the
following, starting at the root the
repository:

```
mkdir build
cd build
cmake ..
make -j8
```

The last command assumes you have eight cores, adjust as needed. Once the build
completes, there are two ways to install the application.

If you don't have a PiProbe, reboot your Pico while holding the "Bootsel"
button, then copy or drag the generated UF2 file `pico-explorer-ir.uf2` onto the
USB drive that appears.

If you have a PiProbe, you can install the program without resetting your Pico
using a command like:

```
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program pico-explorer-ir.elf verify reset exit"
```

## Usage

Once you've built and installed the binary, the display on the Pico Explorer
Base will show three dots while it is waiting for IR input.

Point an IR remote at the receiver, and send a code.  The screen should update
with information about the code received.

The background and text colour will also change to match what the party bulb
(see above) would do if it received that code.