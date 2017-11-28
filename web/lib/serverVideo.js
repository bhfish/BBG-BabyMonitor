"use strict";

var socketioVideo = require('socket.io');
var io;
var ffmpeg;
var spawn = require('child_process').spawn;

exports.listen = function(server) {
    io = socketioVideo.listen(server);
	io.set('log level 1');
	
	io.sockets.on('connection', function(socketioVideo) {
		streamer(socketioVideo);
	});
	io.sockets.on('disconnect', function(socketioVideo) {
		if(ffmpeg !== undefined){
			ffmpeg.kill();
			ffmpeg = undefined;
		}
	});
}


var streamer = function (socketioVideo) {	
	
	if (ffmpeg == undefined){
		ffmpeg = spawn("/usr/bin/ffmpeg", ["-re","-y","-i", "udp://127.0.0.1:1234", "-f", "mjpeg", "-s","640x480","pipe:1"]);
	}

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
		socketioVideo.emit('render',frame);
	});

};
