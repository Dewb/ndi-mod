# ndi-mod for norns

a [norns](https://monome.org/norns) mod to share the contents of the LCD screen in near-real-time via the [NDI](https://streamgeeks.us/what-is-ndi/) streaming video protocol.

<img src="https://user-images.githubusercontent.com/712405/174466864-cbf723bb-a657-4dab-bdf0-572e31b3e7ab.png" width=600>

(*[acrostic by infinitedigits](https://github.com/schollz/acrostic)*)

<img src="https://user-images.githubusercontent.com/712405/174466636-d860d066-bd27-47da-b93f-035291ee5ab8.png" width=600>

(*[takt by itsyourbedtime](https://github.com/itsyourbedtime/takt)*)

<img src="https://user-images.githubusercontent.com/712405/174505098-a66b4830-e427-4ef1-978b-4302c11a8478.png" width=600>

(*[nydl by sixolet](https://github.com/sixolet/nydl)*)


## why ndi / why not ndi?

NDI is fast, and includes zeroconf-based discovery of stream sources. Many popular streaming and video art tools support discovering and receiving NDI streams, including [OBS Studio](https://obsproject.com/)\*, [Resolume](https://resolume.com/), and [Max/MSP/Jitter](https://cycling74.com/products/max-features)\*, and [Touch Designer](https://derivative.ca/UserGuide/NDI).

(*\* requires a plugin*)

Using NDI's standard latency mode, a stream over a local wi-fi network is about a frame behind the norns screen. So far, in informal testing with a Pi CM3+ factory norns, using this mod appears to increase the CPU load by just 1-2%. When using clients like OBS that support NDI's optional low-latency mode, the network update can be almost simultaneous with the norns screen update.

One disadvantage of NDI is that it's not a completely open, unencumbered standard (but it is at least royalty-free.)

Alternatives to NDI include RTMP, HLS, or SRT; those may be better for certain purposes and the general structure of this mod could be adapted to other protocols. 

# how to use

## installing and activating

1. From the maiden console, enter:
   ```
   ;install https://github.com/Dewb/ndi-mod/releases/download/latest/ndi-mod.zip
   ```
2. In the norns menu, navigate to to **SYSTEM > MODS**, scroll to **NDI-MOD**, and turn enc 3
   clockwise to add a `+` next to the mod name. Hit button 2 to back out, and select
   **SYSTEM > RESTART** to relaunch with the mod loaded.
   
This will add about 3MB of files to `~/dust/code/ndi-mod`. To uninstall everything, disable the mod by reversing the above process, then **SYSTEM > RESTART** again. Then, go to the maiden library, scroll to `ndi-mod` in the installed section, and click **remove**.

## documentation

* [User's Guide](docs/User's%20Guide.md) - viewing NDI streams with [OBS Studio](https://obsproject.com/), [Resolume](https://resolume.com/), [Max/MSP/Jitter](https://cycling74.com/products/max-features), [Touch Designer](https://derivative.ca/UserGuide/NDI), and mobile devices.
* [Scripting](docs/Scripting.md) - customizing the mod behavior from norns scripts and/or the maiden console, including using offscreen images to create additional NDI streams.
* [Developing](docs/Developing.md) - building and hacking on the mod

# license and copyright

NDI is a trademark of NewTek and the NDI SDK is copyrighted by NewTek. It is included here under the terms of the [NDI SDK License Agreement](dep/ndi/Processing.NDI.Lib.Licenses.txt). The NDI source headers are licensed with [The MIT License](https://mit-license.org/).

The rest of this repo is also licensed under [The MIT License](https://mit-license.org/). This software is provided as-is, without warranty of any kind, use at your own risk.

