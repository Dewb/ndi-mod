## ndi-mod User's Guide

**Contents:**
* [General Tips](#general-tips)
* [Advanced Topics](#advanced-topics)
* [Using NDI output with desktop software](#using-ndi-output-with-desktop-software)
   * [NDI Studio Monitor](#viewing-ndi-output-with-ndi-studio-monitor)
   * [OBS Studio](#obs-studio)
   * [Resolume Avenue & Arena](#resolume-avenue--arena)
   * [Max/MSP/Jitter](#maxmspjitter)
   * [Touch Designer](#touch-designer)
* [Mobile devices](#mobile-devices)

## General Tips

* The norns should show up as "**NORNS (screen)**" in any NDI-supporting app or device that can reach norns through the network. NDI 5 support is best, but NDI 4 apps might also be able to connect.
* If, in any program, you see local NDI sources but you don't see **NORNS**, make sure the app is allowed through any firewalls, and restart the app.

## Advanced Topics

Read about customizing the behavior of your NDI streams from a norns script in [Scripting](Scripting.md).

Visit [Developing](Developing.md) for information about building and hacking on ndi-mod itself.

## Using NDI output with desktop software

### NDI Studio Monitor
*(PC/Mac/Linux)*

The NDI Studio Monitor tool provided by NewTek can view and record NDI streams.

1. Install the [NDI Tools](https://ndi.tv/tools/). (This requires giving an e-mail address to NewTek.)
2. Run NDI Studio Monitor and click the three-lines icon in the upper left corner.
3. You should see **NORNS** in the sources list, click it and select **screen** from the flyout.

### OBS Studio
*(PC/Mac/Linux)*

OBS can stream, record, and composite NDI streams with other sources when both the [obs-ndi](https://github.com/Palakis/obs-ndi) plugin and the [NewTek NDI Tools](https://ndi.tv/tools/) are installed.

1. Install [OBS Studio](https://obsproject.com/).
2. Install the [NDI plugin for OBS Studio](https://github.com/Palakis/obs-ndi/releases).
3. Install the [the latest NDI Tools](https://ndi.tv/tools/). (NOTE: The Windows installer will install version 4 of the tools, so if you'd already installed them previously you may need to *re-install* NDI 5 after running the plugin installer. If you install the OBS plugin with the Windows .zip build, or the MacOS or Linux builds, you should only need to install the NDI 5 Tools once.)
5. Run OBS Studio. If you get a firewall prompt (Windows) allow OBS to contact devices on the local network.
6. Click the plus below the **Sources** list to add a new source. **NDI™ Source** should be in the list. Select it and then hit **OK**.
7. In the Properties window, click the **Source Name** dropdown and choose **NORNS (screen)**.
8. Hit **OK**. You should see the norns screen show up in the canvas. Click and drag the window and its corner controls to move and resize it.
9. *Optional but recommended*:
   * To keep your pixels crisp and blocky, right-click the NDI source and set **Scale Filtering** to **Point**.
   * For the best performance, open the **Properties** window for the NDI source, scroll down to **Latency Mode** and change it to **Low (experimental)**.

**Tips**
* If you want to overlay the screen over your webcam or other video sources:
   * Select the source in the **Sources** list, click **Filters**, click the plus under **Effect Filters**, and choose **Luma Key**.
   * Set **Luma Max** to `1.0`, **Luma Min** to `0.002`, and both **Smooth** values to `0.0`.
* If you want to adjust the brightness levels (which are going to have a different curve than the norns hardware screen) add a **Color Correction** filter and adjust **Gamma**, **Brightness**. To turn the grayscale palette into a color gradient, try **Color Add** in the Color Correction filter or add a **Apply LUT** filter.

<img src="https://user-images.githubusercontent.com/712405/174466636-d860d066-bd27-47da-b93f-035291ee5ab8.png" width=600>

<img src="https://user-images.githubusercontent.com/712405/174505098-a66b4830-e427-4ef1-978b-4302c11a8478.png" width=600>

## Resolume Avenue & Arena

*(PC/Mac)*

Both versions of Resolume's VJ software natively support NDI streams as a source without installing additional software.

1. Start Resolume. If you get a firewall prompt (Windows) allow Resolume to contact devices on the local network.
2. Scroll to the end of the **Sources** tab; you should see **NORNS** listed under the **NDI SERVERS** heading. Drag it into a clip.
3. For more information, [read the NDI section of the Resolume documentation](https://resolume.com/support/en/NDI_inputs_and_outputs).

**Tips**
   * To overlay the norns screen over other video, add an **Auto Mask** effect to the norns screen clip and set the **Contrast** all the way up to 1. A **Bright.Contrast** effect before and/or after **Auto Mask** will allow you to fine tune the results.
   * To use the norns screen as a key for other video, put the norns screen clip in a layer set to the **50 Mask** blend mode. Again, adding a **Bright.Contrast** effect to the clip will allow you to tune the results.

<img src="https://user-images.githubusercontent.com/712405/174467144-db6121e9-5bfe-4919-b2a0-fdb45b3ec37f.png" width=600>

### Max/MSP/Jitter

*(PC/Mac)*

You can use NDI sources as Jitter textures in Max 8.2.0 or later with the [jit.ndi](https://github.com/pixsper/jit.ndi) extension package.

1. Install the [jit.ndi](https://github.com/pixsper/jit.ndi) package following [the instructions in its README](https://github.com/pixsper/jit.ndi#installation).
2. Run Max. Add a `jit.ndi.receive~` object. Right-click the object and choose **Open jit.ndi.receive~ Help**.
3. In the help/example patch, follow the numbered instructions to **Select an NDI source** (choose NORNS) and **Toggle on qmetro.** You should see the norns screen in the output object.

### Touch Designer

*(PC/Mac)*

Touch Designer includes support for NDI 5 out of the box in versions 2022.20000 and later.

1. Run Touch Designer. Hit **Tab** to add an operator, choose the **TOP** tab, and add an **NDI In** operator.
2. If you get a firewall prompt (Windows) allow TD to contact devices on the local network.
3. In the properties of the **NDI In** operator, change **Source Name** to **NORNS**.
4. You should see the norns screen inside the NDI operator. For more information, read the [NDI section in the Touch Designer user guide](https://derivative.ca/UserGuide/NDI).

## Mobile devices

*iOS*

1. Install [NDI Monitor by Sienna/Mark Gilbert](https://apps.apple.com/us/app/ndi-monitor/id1196221514) from the App Store ($9.99).
2. Run it and grant permissions to contact devices on your network.
3. Select **NORNS** from the list of detected sources.

![image](https://user-images.githubusercontent.com/712405/174466657-bc22a195-f5f0-4721-bdcf-acd55813ff5d.png)

*Android*

TBD, no verified solution yet. Nivadema NDI Viewer purports to be a possible option.
