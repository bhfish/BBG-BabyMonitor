"use strict";

var socketio = require('socket.io');
var io;

exports.listen = function(server) {
    io = socketio.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socketio) {
		streamer(socketio);
	});
}


var streamer = function (socketio) {	
	
	var ffmpeg = require('child_process').spawn("/usr/bin/ffmpeg", ["-re","-y","-i", "udp://127.0.0.1:1234", "-f", "mjpeg", "-s","640x480","pipe:1"]);
	
	ffmpeg.on('error', function (err) {
		console.log('ffmpeg error:'+err);
	});

	ffmpeg.on('close', function (code) {
		console.log('ffmpeg exited with code ' + code);
	});

	ffmpeg.stderr.on('data', function (data) {
		console.log('stderr: ' + data);
	});

	ffmpeg.stdout.on('data', function (data) {
		
		var frame = new Buffer(data).toString('base64');
		socketio.emit('render',frame);
	});

};
