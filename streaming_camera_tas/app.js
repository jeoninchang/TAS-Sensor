/**
 * Created by ryeubi on 2015-08-31.
 */

var net = require('net');
var util = require('util');
var fs = require('fs');
var xml2js = require('xml2js');


var sh_timer = require('./timer');
//var sh_serial = require('./serial');


// for streaming
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);
var path = require('path');

var proc;

var usecomport = '';
var usebaudrate = '';
var useparentport = '';
var useparenthostname = '';

var upload_arr = [];
var download_arr = [];

var img_flag = 0 ;

var cameraOptions = {
  width : 600,
  height : 420,
  mode : 'timelapse',
  awb : 'off',
  encoding : 'jpg',
  output : 'stream/camera.jpg',
  q : 50,
  timeout : 10000,
  timelapse : 1000,
  nopreview : true,
  th : '0:0:0'
};

var camera = new require('raspicam')(cameraOptions) ;
camera.start() ;

camera.on('exit', function() {
    //io.sockets.emit('liveStream', 'stream/camera.jpg?_t=' + (Math.random() * 100000));
    camera.stop() ;
    console.log('Restart camera') ;
    camera.start() ;
  }) ;

camera.on('read', function() {
    img_flag = 1 ;
  }) ;


app.use('/', express.static(path.join(__dirname, 'stream')));


app.get('/', function(req, res) {
  res.sendFile(__dirname + '/index.html');
});

var sockets = {};

io.on('connection', function(socket) {

  sockets[socket.id] = socket;
  console.log("Total clients connected : ", Object.keys(sockets).length);

  socket.on('disconnect', function() {
    delete sockets[socket.id];

    // no more sockets, kill the stream
    if (Object.keys(sockets).length == 0) {
      app.set('watchingFile', false);
      if (proc) proc.kill();
      fs.unwatchFile('./stream/camera.jpg');
    }

  });

  socket.on('start-stream', function() {
    startStreaming(io);
  });

});


http.listen(3000, function() {
  console.log('listening on *:3000');
});


function stopStreaming() {

  if (Object.keys(sockets).length == 0) {
    app.set('watchingFile', false);
    if (proc) proc.kill();
    fs.unwatchFile('./stream/camera.jpg');
  }
}

function startStreaming(io) {
//    io.sockets.emit('liveStream', 'camera.jpg?_t=' + (Math.random() * 100000));

  if (app.get('watchingFile')) {
    io.sockets.emit('liveStream', 'camera.jpg?_t=' + (Math.random() * 100000));
    return;
  }

  console.log('Watching for changes...');

  app.set('watchingFile', true);

  fs.watchFile('./stream/camera.jpg', function(current, previous) {
    io.sockets.emit('liveStream', 'camera.jpg?_t=' + (Math.random() * 100000));
  })

}


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

                //sh_serial.open(usecomport, usebaudrate);
            }
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
                            //sh_serial.g_down_buf = cin;
                            //sh_serial.serial_event.emit('down');
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
    if((tick_count % 12) == 0) {
        if (tas_state == 'upload') {

            try{

            //var fileName = 'image_' + tick_count + '.jpg';
            //var fileName = 'camera.jpg';
            var fileName = 'stream/camera.jpg';
            
            // convert image to base64 encoded string
            var base64str = base64_encode(fileName);
            //console.log(base64str);
            console.log('OK!');

            // convert base64 string back to image 
            //base64_decode(base64str, 'copy.jpg');

            var con = base64str;
            //var con = base64_encode(fileName);
            for (var i = 0; i < upload_arr.length; i++) {
                if (upload_arr[i].id == 'timer') {
console.log('check');
                    var cin = {ctname: upload_arr[i].ctname, con: con};
                    //console.log(JSON.stringify(cin) + ' ---->');
                    console.log(upload_arr[i].ctname + ' ---->');
                    upload_client.write(JSON.stringify(cin));
                    break;
                }
            }
            }catch (exception){
                   console.log(exception);
            }
        }
    }

    if((tick_count % 8) == 0) {
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

/*
sh_serial.serial_event.on('up', function () {
    if(tas_state == 'upload') {
        console.log(sh_serial.g_sink_buf);

        // parsing sensor data, manage id according with ctname
        var sink_str = util.format('%s', sh_serial.g_sink_buf);
        var sink_obj = JSON.parse(sink_str);

        for(var i = 0; i < upload_arr.length; i++) {
            if(upload_arr[i].id == sink_obj.id) {
                var cin = {ctname: upload_arr[i].ctname, con: sink_obj.con};
                upload_client.write(JSON.stringify(cin));
                break;
            }
        }
    }
});
*/

// function to encode file data to base64 encoded string
function base64_encode(file) {
    // read binary data
    var bitmap = fs.readFileSync(file);
    // convert binary data to base64 encoded string
    return new Buffer(bitmap).toString('base64');
}

// function to create file from base64 encoded string
function base64_decode(base64str, file) {
    // create buffer object from base64 encoded string, it is important to tell the constructor that the string is base64 encoded
    var bitmap = new Buffer(base64str, 'base64');
    // write buffer to file
    fs.writeFileSync(file, bitmap);
    console.log('******** File created from base64 encoded string ********');
}

