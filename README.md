# Introducing hackaBLE v2.2 

![hackable2](docs/hackaBLEv2.2/hack_front.jpg)

**Whats new**

hackaBLE 2 is a redesigned hackaBLE that used the BLE SoC directly rather than a module. Her's the footprint. 

![hackable2](docs/hackaBLEv2.2/hackaBLE_Dimensions.png)

CAUTION

hackaBLE 2 does not come with a regulator, since it's designed for low power applications. MAX input to VDD is 3.6 V.

![pinout](docs/hackaBLEv2.2/hackable_pinout.png)

**usage**

Programming and integrating hackaBLE to your project is similar as before. Please follow the direction given below.

# hackaBLE

![hackaBLE](docs/hackaBLEv0.2/hackaBLE1.jpg)

**hackaBLE** is a tiny (~ 18 mm x 28 mm) Open Source Nordic nRF52832 based BLE development board you can embed in your BLE projects. It's designed such that you can use it three ways:

- On a breadboard
- On a custom PCB, hand-soldered easily using the castellated 2.54 mm pitch headers
- On a custom PCB, using a stencil and oven, making use of extra pads underneath the PCB

hackaBLE uses offers more value than just using the BLE module directly - since it incorporates the necessary passive components - including the ones for the buck converter for power saving - and adds an RGB LED and a button for convenience. It's also much easier to solder than the bare modules. 

### Pinout for hackaBLE

![pinout](docs/hackaBLEv0.2/hackaBLE-pinout.png)

## PCB Footprint for hackaBLE

![pinout](docs/hackaBLEv0.2/hackaBLE-dims.png)

(The kicad footprint for hackaBLE is readily available in this repository.)

## Programming hackaBLE

![hackaBLE](docs/hackaBLEv0.2/hackaBLE-prog1.jpg)

hackaBLE uses the Nordic nRF52832 which in turn is based on an ARM Cortex-M4 core. So you can really program it with any ARM compatible programmer. We do have a convenient solution though, in the form 
of our [Bumpy][3] blackmagic probe compatible SWD debugger and our [PogoProg][4]. 

![hackaBLE](docs/hackaBLEv0.2/hackaBLE-prog2.jpg)

As shown above, use the four SWD pins of PogoProg on hackaBLE to upload or debug hackaBLE.

Please [read the bumpy documentation][3] on using Bumpy to program hackaBLE.

For more details on nRF5 2programming, please read our guide on [getting started with Nordic nRF5 SDK][1].

## hackaBLE & Arduino

Hackable does not ship with a bootloader, but it is supported in arduino. Please see https://github.com/electronut/ElectronutLabs-bluey#Arduino for details (but select Hackable in the boards menu).

## Buy a hackaBLE!

hackaBLE is available for purchase from our [Tindie store][2]. Please email us at **info@electronut.in** if you have any questions.

<a href="https://www.tindie.com/stores/ElectronutLabs/?ref=offsite_badges&utm_source=sellers_ElectronutLabs&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>

[1]: https://github.com/electronut/ElectronutLabs-bluey/blob/master/nrf5-sdk-setup.md
[2]: https://www.tindie.com/stores/ElectronutLabs/
[3]: https://github.com/electronut/ElectronutLabs-Bumpy
[4]: https://github.com/electronut/ElectronutLabs-PogoProg
 
 
