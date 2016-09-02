/**
 * Created by ryeubi on 2015-08-31.
 */

var net = require('net');
var util = require('util');
var fs = require('fs');
var xml2js = require('xml2js');


var sh_timer = require('./timer');

var usecomport = '';
var usebaudrate = '';
var useparentport = '';
var useparenthostname = '';
var usebluezport = '';
var uselocal = '127.0.0.1';

var upload_arr = [];
var download_arr = [];

var buffers = {};

// This is an async file read
fs.readFile('conf.xml', 'utf-8', function (err, data) {
    if (err) {
        console.log("FATAL An error occurred trying to read in the file: " + err);
        console.log("error : set to default for configuration")
    }
    else {
        var parser = new xml2js.Parser({explicitArray: false});
        parser.parseString(data, function (err, result) {
            if (err) {
                console.log("Parsing An error occurred trying to read in the file: " + err);
                console.log("error : set to default for configuration")
            }
            else {
                var jsonString = JSON.stringify(result);
                conf = JSON.parse(jsonString)['m2m:conf'];

                usecomport = conf.tas.comport;
                usebaudrate = conf.tas.baudrate;
                useparenthostname = conf.tas.parenthostname;
                useparentport = conf.tas.parentport;

		// bluez port
		usebluezport = conf.bluez.parentport;

                if(conf.upload != null) {
                    if (conf.upload['ctname'] != null) {
                        upload_arr[0] = conf.upload;
                    }
                    else {
                        upload_arr = conf.upload;
                    }
                }

                if(conf.download != null) {
                    if (conf.download['ctname'] != null) {
                        download_arr[0] = conf.download;
                    }
                    else {
                        download_arr = conf.download;
                    }
                }

            }
        });

	net.createServer(function (socket) {
                socket.id = Math.random() * 1000;

                buffers[socket.id] = '';

                socket.on('data', function(data) {
                        // 'this' refers to the socket calling this callback.
                        buffers[this.id] += data.toString();

                        if(buffers[this.id] != ''){
                                tas_man_count++;
                        }

                        if (tas_state == 'upload') {
                                for (var i = 0; i < upload_arr.length; i++) {
                                        var con = buffers[this.id];

                                        var cin = {ctname: upload_arr[i].ctname, con: con};

                                        console.log(JSON.stringify(cin) + ' ---->');

                                        upload_client.write(JSON.stringify(cin));
                                 }
                        }else if(tas_state == 'connect' || tas_state == 'reconnect') {
                                upload_client.connect(useparentport, useparenthostname, function() {
                                        console.log('upload Connected');
                                        tas_man_count = 0;

                                        for (var i = 0; i < download_arr.length; i++) {
                                                console.log('download Connected - ' + download_arr[i].ctname + ' hello');
                                                var cin = {ctname: download_arr[i].ctname, con: 'hello'};
                                                upload_client.write(JSON.stringify(cin));

                                                if (tas_man_count >= download_arr.length) {
                                                        tas_state = 'upload';
                                                }
                                        }
                               });
                        }
                });

                socket.on('end', function() {
                        console.log('end');
                    });
                    socket.on('close', function() {
                        console.log('close');
                    });
                    socket.on('error', function(e) {
                        console.log('error ', e);
                    });
                    //socket.write('hello from tcp server');

       }).listen(usebluezport, function() {
                    console.log('TCP Server ( '+ uselocal+ ' ) is listening on port ' + usebluezport);
       });
    }
});


var tas_state = 'connect';

var upload_client = new net.Socket();
//upload_client.connect(parent_port, '127.0.0.1', function() {
//    console.log('upload Connected');
//    for (var i = 0; i < download_arr.length; i++) {
//        var cin = {ctname: download_arr[i].ctname, con: 'hello'};
//        upload_client.write(JSON.stringify(cin));
//    }
//    tas_state = 'reconnect';
//});

upload_client.on('data', function(data) {
    //client.destroy(); // kill client after server's response

    if (tas_state == 'connect' || tas_state == 'reconnect' || tas_state == 'upload') {
        var data_arr = data.toString().split('}');
        for(var i = 0; i < data_arr.length-1; i++) {
            var line = data_arr[i];
            line += '}';
            var sink_str = util.format('%s', line.toString());
            var sink_obj = JSON.parse(sink_str);

            if (sink_obj.ctname == null || sink_obj.con == null) {
                console.log('Received: data format mismatch');
            }
            else {
                if (sink_obj.con == 'hello') {
                    console.log('Received: ' + data);

                    if (++tas_man_count >= download_arr.length) {
                        tas_state = 'upload';
                    }
                }
                else {
                    for (var j = 0; j < upload_arr.length; j++) {
                        if (upload_arr[j].ctname == sink_obj.ctname) {
                            console.log('ACK : ' + line + ' <----');
                            break;
                        }
                    }

                    for (j = 0; j < download_arr.length; j++) {
                        if (download_arr[j].ctname == sink_obj.ctname) {
                            cin = JSON.stringify({id: download_arr[i].id, con: sink_obj.con});
                            break;
                        }
                    }
                }
            }
        }
    }
});

upload_client.on('error', function(err) {
    tas_state = 'reconnect';
});

upload_client.on('close', function() {
    console.log('Connection closed');
    upload_client.destroy();
    tas_state = 'reconnect';
});


var count = 0;
var tick_count = 0;
var tas_man_count = 0;
sh_timer.timer.on('tick', function() {
    tick_count++;
    if((tick_count % 2) == 0) {
        if (tas_state == 'upload') {
            var con = 'TAS' + count++ + ',' + '55.2';
            for (var i = 0; i < upload_arr.length; i++) {
                if (upload_arr[i].id == 'timer') {
                    var cin = {ctname: upload_arr[i].ctname, con: con};
                    console.log(JSON.stringify(cin) + ' ---->');
                    upload_client.write(JSON.stringify(cin));
                    break;
                }
            }
        }
    }

    if((tick_count % 3) == 0) {
        if(tas_state == 'connect' || tas_state == 'reconnect') {
            upload_client.connect(useparentport, useparenthostname, function() {
                console.log('upload Connected');
                tas_man_count = 0;
                for (var i = 0; i < download_arr.length; i++) {
                    console.log('download Connected - ' + download_arr[i].ctname + ' hello');
                    var cin = {ctname: download_arr[i].ctname, con: 'hello'};
                    upload_client.write(JSON.stringify(cin));
                }

                if (tas_man_count >= download_arr.length) {
                    tas_state = 'upload';
                }
            });
        }
    }
});
