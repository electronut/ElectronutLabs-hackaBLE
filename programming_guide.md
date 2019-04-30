 

hackaBLE does not ship with a bootloader, but it is supported in Arduino. Please visit [here](https://github.com/electronut/ElectronutLabs-bluey#Arduino) for details (but select hackaBLE in the board's menu).

[Here's](https://electronut.in/using-hackable-with-arduino/) a blog article which will help you create a BLE custom characteristic and program it to hackaBLE using Arduino IDE.

The detailed hackaBLE programming guide comming soon.  

## Using Web Bluetooth

You can connect directly to hackaBLE using [Web Bluetooth](https://webbluetoothcg.github.io/web-bluetooth/). For this to work, your browser must support Web Bluetooth. To check list of available browsers, visit this [link](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API#Browser_compatibility). On Google Chrome, you should have enabled experimental web features by visiting the url :
```chrome://flags/#enable-experimental-web-platform-features```

### Try it here !

This demo shows temperature and humidity data received over BLE from sensor attached to hackaBLE. You can access the Firmware at this [link](https://gitlab.com/electronutlabs-public/electronutlabs-hackable/tree/master/code/zephyr_ble_th).

<p id="status_p"></p>
<button data-md-color-primary="light-blue" id="connect">Connect</button>
<br>
<p>
    Temperature : <span id="temp_text"></span>
</p>
<canvas style="border: 1px solid black;" id="temperature" width="400" height="100"></canvas>
<br>
<p>
    Humidity : <span id="hum_text"></span>
</p>
<canvas style="border: 1px solid black;" id="humidity" width="400" height="100"></canvas>
<script src="https://cdnjs.cloudflare.com/ajax/libs/smoothie/1.34.0/smoothie.js"></script>
<script src="../code/web-bluetooth-demo/index.js"></script>