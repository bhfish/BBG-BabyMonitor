"use strict"

const http = require('http');
const fs   = require('fs');
const path = require('path');
const mime = require('mime');

const HOST_SERVER_LISTEN_PORT = 8088;

var server = http.createServer(function(request, response) {
    var filePath = false;

    if (request.url == '/') {
        filePath = 'public/index.html';
    } else {
        filePath = 'public' + request.url;
    }

    var absPath = './' + filePath;
    serveStatic(response, absPath);
});

server.listen(HOST_SERVER_LISTEN_PORT, function() {
    console.log("Server listening on port " + HOST_SERVER_LISTEN_PORT);
});

function serveStatic(response, absPath) {
    fs.exists(absPath, function(exists) {
        if (exists) {
            fs.readFile(absPath, function(err, data) {
                if (err) {
                    send404(response);
                } else {
                    sendFile(response, absPath, data);
                }
            });
        } else {
            send404(response);
        }
    });
}

function send404(response) {
    response.writeHead(404, {'Content-Type': 'text/plain'});
    response.write('Error 404: resource not found.');
    response.end();
}

function sendFile(response, filePath, fileContents) {
    response.writeHead(
            200,
            {"content-type": mime.lookup(path.basename(filePath))}
        );
    response.end(fileContents);
}

var beatboxServer = require('./lib/serverLib.js');
beatboxServer.listen(server);