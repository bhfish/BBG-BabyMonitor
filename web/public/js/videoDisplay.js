"use strict"

//$(function(){
$(document).ready(function(){
		
	var iosocket = io.connect('http://192.168.7.2:8088');
	iosocket.on('connect', function () {
			
                $('#video-status').html($('<li>Connected</li>'));
				
		iosocket.on('disconnect', function() {
                    $('#video-status').html($('<li>Disconnected</li>'));
                });
				
                iosocket.on('message', function(message) {
                    $('#video-status').html($('<li></li>').text(message));
					console.log(message);
                });
			
		iosocket.on('render', function(data) {
			try {
				//console.log(data);
				console.log('Frame Arrived');
				var canvas = document.getElementById('videoCanvas');
				var context = canvas.getContext('2d');
				var imageObject = new Image();
				imageObject.src = 'data:image/jpeg;base64,' + data;
				imageObject.onload = function(){
					context.height = imageObject.height;
					context.width = imageObject.width;
					console.log("frame width: "+imageObject.width +", height: "+imageObject.height);
					//console.log(imageObject.height);
					context.drawImage(imageObject,0,0,context.width,context.height);
				}		
			} catch(e){
				console.log(e); 
			}
		});				
                
	});
            
});

