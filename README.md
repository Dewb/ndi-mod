## norns-ndi-mod

norns system mod to share screen via the [NDI](https://streamgeeks.us/what-is-ndi/) streaming video standard. 

## why ndi / why not ndi?

NDI has the advantages of easy discovery and configuration in popular tools like [OBS Studio](https://obsproject.com/) and [Resolume](https://resolume.com/). 

It has the disadvantage of being not an open standard (but it is at least royalty-free.)

## todo

- [X] ~~basic scaffolding~~
- [ ] proof of concept: send a static 128x64 test card from norns
- [ ] mvp: send a screen buffer from a manual lua call in `redraw()`
- [ ] stretch: send screen buffer automatically on every update (likely will require core changes to add a new mod hook for screen pre-draw)
- [ ] documentation & demo video

## how to use

*TODO: instructions for end user install. goals: no external dependencies via apt/etc., installable via maiden ;install command*

## building from source

to build and copy into ~/dust/code:

```bash
./build.sh
```

## references/thanks

* https://github.com/ngwese/norns-event-demo
* https://github.com/raspberry-pi-camera/raspindi

## license and copyright

NDI is a trademark of NewTek and the NDI SDK is copyrighted by NewTek. It is included here (in `deps/ndi`) under the terms of the [NDI SDK License Agreement](https://233b1d13b450eb6b33b4-ac2a33202ef9b63045cbb3afca178df8.ssl.cf1.rackcdn.com/license/NDI-SDK-License-Agreement-2019.pdf). The NDI source headers are licensed with [The MIT License](https://mit-license.org/).

The rest of this repo is licensed under [version 3 of the GPL](https://www.gnu.org/licenses/gpl-3.0.en.html).
