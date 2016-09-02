/**
 * Created by ryeubi on 2015-08-31.
 */

var util = require('util');
var os = require('os');
var serialport = require('serialport');

var SerialPort;
var myPort;

var portStatus = "close";


exports.open = function(portname, baudrate) {
    SerialPort = serialport.SerialPort;

    myPort = new SerialPort(portname, {
        baudRate : baudrate,
        buffersize : 1,
        //parser : serialport.parsers.readline("22")
    });

    myPort.on('open', showPortOpen);
    myPort.on('data', saveLastestData);
    myPort.on('close', showPortClose);
    myPort.on('error', showError);
};



var cur_c = '';
var pre_c = '';
var g_sink_buf = '';
var g_sink_ready = [];
var current_positon = 0;



exports.g_down_buf = '';

exports.serial_event = new process.EventEmitter();
exports.g_sink_buf = g_sink_buf;


// list serial ports:
serialport.list(function (err, ports) {
    ports.forEach(function (port) {
        console.log(port.comName);
    });
});


function showPortOpen() {
    console.log('port open. Data rate: ' + myPort.options.baudRate);
    portStatus = "open";
}

var counter = 0;
var sensorTick = setInterval(function () {
    if(portStatus == "open"){
        try{
            var buf = new Buffer(4);
            buf[0] = 0x11;
            buf[1] = 0x01;
            buf[2] = 0x01;
            buf[3] = 0xED;
            myPort.write(buf);
        }catch(exp) {
            console.log(exp.toString());
        }
    }
}, 2000);

var temp_data = new Array(8);

var count = 0;

function saveLastestData(data) {

    count ++;
    //console.log('length ' + data.length);
    var val = data.readUInt16LE(0, true);
    //console.log('count -> ' + count + ' data-> ' + val );

    g_sink_ready.push(val);

    if(count % 8 == 0){
        //console.log('===================');
        console.log(g_sink_ready);

            var p1 = g_sink_ready[0];
            var p2 = g_sink_ready[1];
            var p3 = g_sink_ready[2];
            var p4 = g_sink_ready[3];
            var p5 = g_sink_ready[4];
            var p6 = g_sink_ready[5];
            var p7 = g_sink_ready[6];
            var p8 = g_sink_ready[7];

            var nValue = p4 * 256 + p5;

        console.log(nValue);

        g_sink_ready = [];

        exports.serial_event.emit('up', nValue);
    }
}

exports.serial_event.on('down', function () {
    console.log(exports.g_down_buf);
    myPort.write(exports.g_down_buf);
});


function showPortClose() {
    console.log('port closed.');
}

function showError(error) {
    var error_str = util.format("%s", error);
    console.log(error.message);
    if (error_str.substring(0, 14) == "Error: Opening") {

    }
    else {
        console.log('Serial port error : ' + error);
    }
}


