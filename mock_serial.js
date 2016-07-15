 /**
  * @file mock_serial.js
  * @author Seung-Chul Kwak [rossi.kwak@gmail.com]
  * @brief Instead of using actual serial device, users can emulate device using this script.
  *		   The implementation relies on the interface defined in the 'serail.js & app.js for TAS v1.3' by ryeubi.
  *		   To use this scropt, simply replace the include statement as following 
  *
  *		   before> var sh_serial = require('./serial.js');
  *		   after> var sh_serial = require('./mock_serial.js');
  */

exports.serial_event = new process.EventEmitter();
exports.g_sink_buf = '';
exports.g_down_buf = '';

exports.open = function(portname, baudrate) {
    console.log('port openend, Portname:'+portname+',Baudrate: ' + baudrate);
};

 /**
  * @brief You can manipulate messages going up from the device to TAS
  */
var timeout = 3000;	//timeout as 3 seconds
setInterval(function () {
	console.log('timer');

	//monitoring values from device to TAS
	exports.g_sink_buf = '{"con":"0.00W, 1.62W","id":"fe80:0000:0000:0000:0212:4b00:0235:bca0"}'; 

	//fire event, this will invoke the waiting event in app.js of TAS 
	exports.serial_event.emit('up');
},timeout);

 /**
  * @brief You can handle data coming from the server via TAS, ex,control command
  */
exports.serial_event.on('down', function () {
	var jsonObj = JSON.parse(exports.g_down_buf);
	console.log(jsonObj);

	if(jsonObj.con=="0"){									//define 0 as OFF
		console.log("===== O F F =====");
	}else if(jsonObj.con=="1"){								//define 1 as ON
		console.log("===== O N =====");
	}else{
		console.log("===== UNDEFINED"+jsonObj.con+" ===");	//otherwise, not defined
	}
});
