var noble = require('noble');
const _cliProgress = require('cli-progress');

var imageData = require("./image")

// create a new progress bar instance and use shades_classic theme
const transfer_progress_bar = new _cliProgress.Bar({}, _cliProgress.Presets.shades_classic);

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
        if (peripheral.advertisement.localName.replace(/\0/g, '') == 'hackBLE') {
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
                                }
                            }

                            send_data(characteristic);
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
    console.log("num_writes", num_writes)
    num_writes--;
    transfer_progress_bar.update(num_writes);

    if(num_writes == 0)
    {
        transfer_progress_bar.stop();
        console.log("done.")
        process.exit(0);
    }
}

function send_data(characteristic)
{
    characteristic.on("write", write_cb)

    var frame_no = 1;
    var num_wr_calls = 0;
    var last_frame_flag = false;
    while(imageData.bwData.length != 0)
    {
        var image_data_frame = imageData.bwData.splice(0,16)
        
        var vnd_value = [];
        var written_length = 0;

        if(image_data_frame.length<16 || (image_data_frame.length==16 && imageData.bwData.length==0))
        {
            vnd_value.push(0) // Last frame prefix:  "0b 0000 0000"
            vnd_value.push(0)
            last_frame_flag = true;
            console.log("last frame")
        }
        else
        {
            vnd_value.push((frame_no & 0xff00)>>8)
            vnd_value.push(frame_no & 0x00ff)
        }

        for(x=0; x<image_data_frame.length; x++)
        {
            vnd_value.push(image_data_frame[x]);
        }

        const vnd_value_buf = Buffer.from(vnd_value)

        //console.log(JSON.stringify(vnd_value))
        console.log(frame_no, vnd_value_buf.toString('hex'))


        function write_to_char(buf)
        {
            characteristic.write(buf, false, function(error){console.log(buf.toString('hex'))})
        }
        
        write_to_char(vnd_value_buf);
        num_wr_calls++;

        if(last_frame_flag)
        {
            num_writes = num_wr_calls;
        }
        
        frame_no++;
    }
}
