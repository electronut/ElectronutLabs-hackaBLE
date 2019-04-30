/** index.js
 *
 * brief: Web Bluetooth demo code for hackaBLE for docs. 
 *
 * COPYRIGHT NOTICE: (c) 2019 Electronut Labs.
 * All rights reserved. 
 */
var connect = document.getElementById('connect');
var temp_text = document.getElementById("temp_text");
var hum_text = document.getElementById("hum_text");
var log = console.log;
var myService, bluetoothDevice;
var smoothie, line1;
var smoothie2, line2;

function onDisconnected(event) {
    // Object event.target is Bluetooth Device getting disconnected.
    console.log('> Bluetooth Device disconnected');
    connect.innerText = "Connect"
    smoothie2.removeTimeSeries(line2);
    smoothie.removeTimeSeries(line1);
    smoothie2.stop();
    smoothie.stop();

    temp_text.innerText = "";
    hum_text.innerText = "";

}

if (!navigator.bluetooth) {
    document.getElementById('status_p').innerText = "This browser is not supported";
    document.getElementById('status_p').style.color = "red";
}

connect.addEventListener('click', function (event) {
    if (connect.innerText == "Connect") {


        navigator.bluetooth.requestDevice({ filters: [{ services: ["environmental_sensing"] }] })
            .then(device => {
                // Assign to bluetoothDevice , to make it global, so that disconnect can be handled
                bluetoothDevice = device;
                bluetoothDevice.addEventListener('gattserverdisconnected', onDisconnected);

                /* ... */
                console.log(device);
                return bluetoothDevice.gatt.connect();
            })
            .then(server => {
                console.log(server);
                return server.getPrimaryService("environmental_sensing");
            })
            .then(service => {
                console.log(service);
                myService = service;
                connect.innerText = "Disconnect"
                myService.getCharacteristic("humidity")
                    .then(characteristic => {
                        console.log(characteristic);
                        return characteristic.startNotifications().then(_ => {
                            log('> Notifications started');
                            /* 
                                On successful connection and receiving notification data, draw graph on canvas
                                and start stream for humidity data, with a delay of 2 sec. Scale of min and max
                                value is set to keep the minimal visual change of humidity data.
                            */
                            characteristic.addEventListener('characteristicvaluechanged',
                                handleHumidityNotifications);
                            smoothie2 = new SmoothieChart({ maxValueScale: 1.001, minValueScale: 1.001 });
                            smoothie2.streamTo(document.getElementById("humidity"), 2000);
                            line2 = new TimeSeries();
                            smoothie2.addTimeSeries(line2);
                        });
                    })
                    .catch(error => { console.log(error); });

                return myService.getCharacteristic("temperature");
            })
            .then(characteristic => {
                console.log(characteristic);
                return characteristic.startNotifications().then(_ => {
                    log('> Notifications started');
                    /* 
                        On successful connection and receiving notification data, draw graph on canvas
                        and start stream for temperature data, with a delay of 2 sec. Scale of min and max
                        value is set to keep the minimal visual change of temperature data.
                    */
                    characteristic.addEventListener('characteristicvaluechanged',
                        handleTemperatureNotifications);
                    smoothie = new SmoothieChart({ maxValueScale: 1.001, minValueScale: 1.001 });
                    smoothie.streamTo(document.getElementById("temperature"), 2000);
                    line1 = new TimeSeries();
                    smoothie.addTimeSeries(line1);
                });
            })
            .catch(error => { console.log(error); });

    } else {

        if (!bluetoothDevice) {
            return;
        }
        console.log('Disconnecting from Bluetooth Device...');
        if (bluetoothDevice.gatt.connected) {
            bluetoothDevice.gatt.disconnect();
        } else {
            console.log('> Bluetooth Device is already disconnected');
        }
    }

    /* 
        on successful notification through temp and humidty, data is shown on the screen
        and also updated to the graph.
    */
    function handleTemperatureNotifications(event) {
        var temperature_bytes = new Int16Array(event.target.value.buffer);
        line1.append(new Date().getTime(), temperature_bytes[0] * 0.01);
        temp_text.innerText = (temperature_bytes[0] * 0.01).toFixed(2) + " °C";
    }

    function handleHumidityNotifications(event) {
        var humidity_bytes = new Uint16Array(event.target.value.buffer);
        line2.append(new Date().getTime(), humidity_bytes[0] * 0.01);
        hum_text.innerText = (humidity_bytes[0] * 0.01).toFixed(2) + " %";

    }

});