var noble = require('noble');
const _cliProgress = require('cli-progress');

var imageData = require("./image")

// create a new progress bar instance and use shades_classic theme
const transfer_progress_bar = new _cliProgress.Bar({}, _cliProgress.Presets.shades_classic);

const no_of_frames = 313;

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

        // check for hackBLE
        if (peripheral.advertisement.localName.replace(/\0/g, '') == 'Papyr') {
            console.log("device found");

            noble.stopScanning();

            //connect to the device
            peripheral.connect(function (error) {
                if (error) throw error;
                else {
                    console.log("connected")

                    // peripheral.discoverAllServicesAndCharacteristics(function (error, services, characteristics) {
                    //     console.log("services >>>> ")
                    //     console.log(services)
                    //     console.log("---------------------------------------------")
                    //     console.log("characteristics >>>> ")
                    //     console.log(characteristics)
                    //     console.log("---------------------------------------------")
                    // });

                    peripheral.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicUUIDs, function (error, services, characteristics) {
                        if (error) throw error;
                        else {
                            for (i = 0; i < characteristics.length; i++) {
                                if (characteristics[i].uuid == characteristicUUIDs[0]) {
                                    characteristic = characteristics[i];
                                    console.log('characteristic uuid found : ', characteristic.uuid);
                                    characteristic.on("write", write_cb)
                                }
                            }
                            console.log("");

                            for(var frame_no=1; frame_no<314; frame_no++)
                            {
                                var vnd_value = [];

                                if(frame_no==no_of_frames)
                                {
                                    vnd_value.push(0) // Last frame prefix:  "0b 0000 0000"
                                    vnd_value.push(0)
                                }
                                else
                                {
                                    vnd_value.push((frame_no & 0xff00)>>8)
                                    vnd_value.push(frame_no & 0x00ff)
                                }


                                var image_data_frame = imageData.bwData.splice(0,16)

                                for(x=0; x<16; x++)
                                {
                                    vnd_value.push(image_data_frame[x]);
                                }

                                const vnd_value_buf = Buffer.from(vnd_value)

                                //console.log(JSON.stringify(vnd_value))
                                // console.log(vnd_value_buf.toString('hex'))

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

var num_writes = 0;
function write_cb()
{
    if(num_writes == 0)
    {
        transfer_progress_bar.start(no_of_frames, num_writes);
    }

    // console.log("num_writes", num_writes)
    num_writes++;
    transfer_progress_bar.update(num_writes);

    if(num_writes == no_of_frames)
    {
        transfer_progress_bar.stop();
        // console.log("done.")
        process.exit(0);
    }
}
