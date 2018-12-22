var noble = require('noble');

var imageData = require("./image")

// service and characteristic
var serviceUUIDs = ['7289140057ee475bbf86cf0898f095d9'];
var characteristicUUIDs = ['7289140157ee475bbf86cf0898f095d9'];

// Start scanning ble devices
noble.on('stateChange', function (state) {
    console.log(state);
    if (state === 'poweredOn') {
        console.log('scanning...');
        noble.startScanning([], false);
    } else {
        console.log('stopped.');
        noble.stopScanning();
    }
});

noble.on('scanStart', function () {
    console.log('scan started');
});

// this event is emitted when any ble device is found.
noble.on('discover', function (peripheral) {
    if (peripheral.advertisement.localName) {
        console.log(peripheral.advertisement.localName);

        // check for grid eye sensor device
        if (peripheral.advertisement.localName.replace(/\0/g, '') == 'Nord') {
            console.log("device found");

            noble.stopScanning();

            //connect to the device
            peripheral.connect(function (error) {
                if (error) throw error;
                else {
                    console.log("connected")

                    // peripheral.discoverAllServicesAndCharacteristics(function (error, services, characteristics) {
                    //     if (error) throw error;
                    //     else {
                    //         for (i = 0; i < characteristics.length; i++) {
                    //             console.log('characteristic uuid found : ', characteristics[i].uuid);
                    //         }
                    //         for (i = 0; i < services.length; i++) {
                    //             console.log('services uuid found : ', services[i].uuid);
                    //         }
                    //     }
                    // });

                    peripheral.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicUUIDs, function (error, services, characteristics) {
                        if (error) throw error;
                        else {
                            for (i = 0; i < characteristics.length; i++) {
                                if (characteristics[i].uuid == characteristicUUIDs[0]) {
                                    characteristic = characteristics[i];
                                    console.log('characteristic uuid found : ', characteristic.uuid);
                                }
                            }
                            console.log("");

                            // var vnd_value = [];
                            // vnd_value.push(192) // Frame 1 prefix: "0b 110X XXXX    XXXX XXXX"
                            // vnd_value.push(0)
                            // console.log(vnd_value)
                            // const vnd_value_buf = Buffer.from(vnd_value)

                            // //console.log(JSON.stringify(vnd_value))
                            // console.log(vnd_value_buf.toString('hex'))
                            
                            // process.exit(0)

                            for(var frame_no=1; frame_no<314; frame_no++)
                            {
                                var vnd_value = [];

                                if(frame_no==313)
                                {
                                    vnd_value.push(0) // Last frame prefix:  "0b 0000 0000"
                                    vnd_value.push(0)
                                }
                                else
                                {
                                    vnd_value.push((frame_no & 0xff00)>>8)
                                    vnd_value.push(frame_no & 0x00ff)
                                }

                                // console.log(imageData.zephyr_bwData.length)

                                var image_data_frame = imageData.qr_bwData.splice(0,16)

                                for(x=0; x<16; x++)
                                {
                                    vnd_value.push(image_data_frame[x]);
                                    // vnd_value.push(Math.floor(Math.random() * Math.floor(256)));
                                }

                                const vnd_value_buf = Buffer.from(vnd_value)

                                //console.log(JSON.stringify(vnd_value))
                                console.log(vnd_value_buf.toString('hex'))

                                characteristic.write(vnd_value_buf, false, function (error) {
                                    if (error) throw error;
                                });
                            }
                        }
                    });
                }
            })
        }
    }
});