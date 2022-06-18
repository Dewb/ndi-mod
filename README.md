## norns-ndi-mod

norns system mod to share screen via the [NDI](https://streamgeeks.us/what-is-ndi/) streaming video standard. 

## why ndi / why not ndi?

NDI has the advantages of easy discovery and configuration in popular tools like [OBS Studio](https://obsproject.com/) and [Resolume](https://resolume.com/). 

It has the disadvantage of being not an open standard (but it is at least royalty-free.)

## todo

- [X] ~~basic scaffolding~~
- [X] ~~proof of concept: send a static 128x64 test card from norns~~
- [X] ~~mvp: send a screen buffer from a manual lua call in `redraw()`~~
- [ ] stretch: send screen buffer automatically on every update (may require core changes to add a new mod hook for screen pre-draw?)
- [ ] documentation & demo video

## how to use

### installing

*TODO: instructions for end user install. goals: no external dependencies via apt/etc., installable via maiden ;install command*

temporary installation & usage:
* you must build from source, see next section
* enable the mod: go to SYSTEM > MODS in the norns menu, scroll to NDI-MOD, and turn enc 3 clockwise to add a + next to the mod. Hit button 2 to back out and select SYSTEM > RESTART to relaunch with the mod loaded.
* call `ndi_mod.start()` in `init()` (or at the maiden console)
* call `ndi_mod.update()` in `redraw()` (or at the maiden console) to send the current screen surface as a NDI frame

beware: this is a proof of concept, does not work reliably yet!

### viewing NDI output on your desktop (PC/Mac/Linux)

The NDI Studio Monitor tool included in NDI Tools can view and record NDI streams.

1. Install the [NDI Tools](https://ndi.tv/tools/). (This requires giving an e-mail address to NewTek.)
2. Run the NDI Studio Monitor and click the three-lines icon in the upper left corner.
3. You should see NORNS in the sources list, click it and select "norns screen" from the flyout.

### using with OBS Studio (PC/Mac/Linux)

OBS Studio is a great open-source tool for streaming or recording video from a variety of sources at once.

1. Install [OBS Studio](https://obsproject.com/).
2. Install the [NDI plugin for OBS Studio](https://github.com/Palakis/obs-ndi/releases).
3. The obs-ndi plugin hasn't been updated in a while and can be finicky. Notably, it's built with an older version of the NDI Tools. You may need to *re-install* the OBS 5 tools after running the obs-ndi installer. Try the OBS forums for more advice if you have trouble.
4. Run OBS Studio. If you get a firewall prompt (Windows) allow OBS to contact devices on the local network.
5. In the Sources list, click the plus to add a new source. "NDI Source" should be in the list. Click it and hit OK.
6. In Source Properties, click the Source Name dropdown and choose NORNS. Hit OK.
7. You should see the norns screen show up in the canvas. Click and drag it and its corner controls to move and resize the window.
8. Right-click the screen element and set Scale Filtering to Point to keep your pixels blocky.
7. If you want to overlay the screen over your webcam or other video sources:
   * click Filters, click the plus under Effect Filters, and choose Luma Key
   * Set Luma Max to 1.0, Luma Min to 0.002, and both Smooth values to 0.0.

### using with Resolume Avenue/Arena

1. Start Resolume. If you get a firewall prompt (Windows) allow Resolume to contact devices on the local network.
2. Scroll to the end of the Sources tab; you should see NORNS listed under the NDI SERVERS heading. Drag it into a clip.
3. From here you have many aesthetic options. Some suggestions:
   * To overlay the norns screen over other video, add an Auto Mask effect to the norns screen clip and set the Contrast all the way up to 1. A Bright.Contrast effect before and/or after Auto Mask will allow you to fine tune the results.
   * To get an "animated text" effect, put the norns screen clip in a layer set to the 50 Mask blend mode. Again, a Bright.Contrast effect will allow you to tune the results.

### using with mobile devices

*iOS*
* Install [NDI Monitor by Sienna/Mark Gilbert](https://apps.apple.com/us/app/ndi-monitor/id1196221514) from the App Store ($9.99).
* Run it and grant permissions to contact devices on your network.
* Select NORNS from the list of detected sources.

*Android*
* No known solution yet

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
