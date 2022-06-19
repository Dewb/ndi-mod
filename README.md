# ndi-mod for norns

norns system mod to share the screen via the [NDI](https://streamgeeks.us/what-is-ndi/) 
streaming video protocol.

<img src="https://user-images.githubusercontent.com/712405/174466864-cbf723bb-a657-4dab-bdf0-572e31b3e7ab.png" width=600>

## why ndi / why not ndi?

NDI is fast, and supports easy discovery of stream sources. Support for NDI is built in
or available via plugin in several popular streaming and video art tools like [OBS Studio](https://obsproject.com/), 
[Resolume](https://resolume.com/), [Touch Designer](https://derivative.ca/UserGuide/NDI), and 
[Max/MSP/Jitter](https://github.com/pixsper/jit.ndi).

It has the disadvantage of being not an open standard (but it is at least royalty-free.)

In low-latency mode, a NDI stream over the local network is less than a frame behind the norns screen. So far, in informal testing with a Pi CM3+ factory norns, using this mod increases the CPU load by just 1-2%.

Alternatives to NDI include RTMP, HLS, or SRT; those may be better for certain purposes and the general structure of this mod could be adapted to other protocols. 

# how to use

### installing and activating

1. From the maiden console, enter:
   ```
   ;install https://github.com/Dewb/ndi-mod/releases/download/latest/ndi-mod.zip
   ```
2. In the norns menu, navigate to to **SYSTEM > MODS**, scroll to **NDI-MOD**, and turn enc 3
   clockwise to add a `+` next to the mod name. Hit button 2 to back out, and select
   **SYSTEM > RESTART** to relaunch with the mod loaded.

### viewing NDI output on your desktop (PC/Mac/Linux)

The NDI Studio Monitor tool included in NDI Tools can view and record NDI streams.

1. Install the [NDI Tools](https://ndi.tv/tools/). (This requires giving an e-mail address to NewTek.)
2. Run the NDI Studio Monitor and click the three-lines icon in the upper left corner.
3. You should see **NORNS** in the sources list, click it and select **norns screen** from the flyout.

### using with OBS Studio (PC/Mac/Linux)

1. Install [OBS Studio](https://obsproject.com/).
2. Install the [NDI plugin for OBS Studio](https://github.com/Palakis/obs-ndi/releases).
3. Install the [the latest NDI Tools](https://ndi.tv/tools/). (NOTE: The Windows installer will install version 4 of the tools, so if you'd already installed them previously you may need to *re-install* NDI 5 after running the plugin installer. If you install the OBS plugin with the Windows .zip build, or the MacOS or Linux builds, you should only need to install the NDI 5 Tools once.)
5. Run OBS Studio. If you get a firewall prompt (Windows) allow OBS to contact devices on the local network.
6. Click the plus below the **Sources** list to add a new source. **NDI™ Source** should be in the list. Select it and then hit **OK**.
7. In the Properties window, click the **Source Name** dropdown and choose **NORNS**.
8. Hit **OK**. You should see the norns screen show up in the canvas. Click and drag the window and its corner controls to move and resize it.

Tips:
* Right-click the screen element and set **Scale Filtering** to **Point** to keep your pixels blocky.
* In the **Properties** window for the NDI source, scroll down to **Latency Mode** and change it to **Low (experimental)**. 
* If you want to overlay the screen over your webcam or other video sources:
   * Select the source in the **Sources** list, click **Filters**, click the plus under **Effect Filters**, and choose **Luma Key**.
   * Set **Luma Max** to `1.0`, **Luma Min** to `0.002`, and both **Smooth** values to `0.0`.
* Add a **Color Correction** filter and adjust **Gamma**, **Brightness**, and **Color Add** to adjust the color palette of the graphics.

<img src="https://user-images.githubusercontent.com/712405/174466636-d860d066-bd27-47da-b93f-035291ee5ab8.png" width=600>

## using with Resolume Avenue/Arena (PC/Mac)

1. Start Resolume. If you get a firewall prompt (Windows) allow Resolume to contact devices on the local network.
2. Scroll to the end of the **Sources** tab; you should see **NORNS** listed under the **NDI SERVERS** heading. Drag it into a clip.

Tips:
   * To overlay the norns screen over other video, add an **Auto Mask** effect to the norns screen clip and set the **Contrast** all the way up to 1. A **Bright.Contrast** effect before and/or after **Auto Mask** will allow you to fine tune the results.
   * To get an "animated text" effect, put the norns screen clip in a layer set to the **50 Mask** blend mode. Again, adding a **Bright.Contrast** effect to the clip will allow you to tune the results.

<img src="https://user-images.githubusercontent.com/712405/174467144-db6121e9-5bfe-4919-b2a0-fdb45b3ec37f.png" width=600>

### using with mobile devices

*iOS*
* Install [NDI Monitor by Sienna/Mark Gilbert](https://apps.apple.com/us/app/ndi-monitor/id1196221514) from the App Store ($9.99).
* Run it and grant permissions to contact devices on your network.
* Select **NORNS** from the list of detected sources.

![image](https://user-images.githubusercontent.com/712405/174466657-bc22a195-f5f0-4721-bdcf-acd55813ff5d.png)

*Android*
* No known solution yet

## advanced usage

### control via maiden and scripts

When the mod is active, the NDI server is always running and sending video. You can customize the behavior with the following Lua methods:
* `ndi_mod.stop()` will stop sending video
* `ndi_mod.start()` will start sending video
* `ndi_mod.send_frame()` will send a single frame, regardless of the start/stop state

### building from source

to build and copy into ~/dust/code:

```bash
./build.sh
```

# references/thanks

* https://github.com/ngwese/norns-event-demo
* https://github.com/raspberry-pi-camera/raspindi

# license and copyright

NDI is a trademark of NewTek and the NDI SDK is copyrighted by NewTek. It is included here under the terms of the [NDI SDK License Agreement](dep/ndi/Processing.NDI.Lib.Licenses.txt). The NDI source headers are licensed with [The MIT License](https://mit-license.org/).

The rest of this repo is also licensed under [The MIT License](https://mit-license.org/). This software is provided as-is, without warranty of any kind, use at your own risk.

