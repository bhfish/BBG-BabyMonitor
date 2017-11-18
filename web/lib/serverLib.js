"use strict"
/*
    nodejs server and C server program which runs in monitor BBG should agree the following message protocols

    GET request from nodejs server to C server
    e.g. getTemperature\n

    SET request from nodejs server to C server
    e.g. setTemperature:<value>\n

    C server responds to GET request from nodejs server
    e.g. getTemperature:<value>\n

    C server responds to SET request from nodejs server
    e.g. setTemperature:ok\n

    data file format:
    <timestamp>,<data value>\n

*/

const fs = require('fs');
const socketio = require('socket.io');
const dgram = require('dgram');
const net = require('net');

// different socket events
const GET_MONITOR_BBG_STATUS_EVENT_NAME = "getMonitorBBGStatus";
const GET_ALARM_BBG_STATUS_EVENT_NAME = "getAlarmBBGStatus";
const GET_TEMPERATURE_EVENT_NAME = "getTemperature";
const GET_DECIBEL_EVENT_NAME = "getDecibel"
const GET_TEMPERATURE_DATASET_EVENT_NAME = "getTemperatureDataset"
const GET_DECIBEL_DATASET_EVENT_NAME = "getDecibelDataset"
const MONITOR_BBG_FAILURE_EVENT_NAME = "monitorSystemFailure"
const ALARM_BBG_FAILURE_EVENT_NAME = "alarmSystemFailure"

const ALARM_BBG_LISTEN_PORT = 12345
const ALARM_BBG_LISTEN_ADDR = "192.168.3.1"
const MONITOR_BBG_LISTEN_PORT = 8809;
const MONITOR_BBG_LISTEN_ADDR = "127.0.0.1";
const DATA_FILE_EXTENSION = ".csv"
const DATA_FILE_FS = "/tmp"
const TEMPERATURE_DATA_FILE_NAME = "temperature"
const SOUND_DATA_FILE_NAME = "sound"
const MONITOR_BBG_SEND_REQUEST_TIME_OUT = 5000;
const ALARM_BBG_SEND_REQUEST_TIME_OUT = 5000;

var eventTimeoutArr = {};
var monitorBBGSocket = dgram.createSocket('udp4');

// TCP socket to alarm BBG
var alarmBBGSocket = new net.Socket();
var io;
var prevTemperatureDataFileContent = undefined;
var prevSoundDataFileContent = undefined;

// wait for the C server program which runs in monitor BBG to send responses to me
exports.listen = function(server) {
    io = socketio.listen(server);
    io.set('log level 1');

    // fired upon a connection from client
    io.sockets.on('connection', function(clientWebSocket) {
        // establish TCP connection to alarm BBG
        alarmBBGSocket.connect(ALARM_BBG_LISTEN_PORT, ALARM_BBG_LISTEN_ADDR);

        // fired upon a connection failure to the alarm BBG
        alarmBBGSocket.on('error', function(err) {
            // render error to client and try to re-establish connection later
            sendMessageToClient(clientWebSocket, ALARM_BBG_FAILURE_EVENT_NAME, null);
            setTimeout(function(){
                alarmBBGSocket.connect(ALARM_BBG_LISTEN_PORT, ALARM_BBG_LISTEN_ADDR);
            }, ALARM_BBG_SEND_REQUEST_TIME_OUT);
        });

        waitForClientUIRequest(clientWebSocket);
        redirectMonitorBBGResponseToClient(clientWebSocket);
        redirectAlarmBBGResponseToClient(clientWebSocket);
    });
};

function waitForClientUIRequest(clientWebSocket) {
    clientWebSocket.on(GET_MONITOR_BBG_STATUS_EVENT_NAME, function() {
        eventTimeoutArr[GET_MONITOR_BBG_STATUS_EVENT_NAME] = setTimeout(function(){
            sendMessageToClient(clientWebSocket, MONITOR_BBG_FAILURE_EVENT_NAME, null);
        }, MONITOR_BBG_SEND_REQUEST_TIME_OUT);

        sendRequestToMonitorBBG(GET_MONITOR_BBG_STATUS_EVENT_NAME + "\n");
    });

    clientWebSocket.on(GET_ALARM_BBG_STATUS_EVENT_NAME, function() {
        eventTimeoutArr[GET_ALARM_BBG_STATUS_EVENT_NAME] = setTimeout(function(){
            sendMessageToClient(clientWebSocket, ALARM_BBG_FAILURE_EVENT_NAME, null);
        }, ALARM_BBG_SEND_REQUEST_TIME_OUT);

        sendRequestToAlarmBBG(GET_ALARM_BBG_STATUS_EVENT_NAME);
    });


    clientWebSocket.on(GET_TEMPERATURE_EVENT_NAME, function() {
        eventTimeoutArr[GET_TEMPERATURE_EVENT_NAME] = setTimeout(function(){
            sendMessageToClient(clientWebSocket, MONITOR_BBG_FAILURE_EVENT_NAME, null);
        }, MONITOR_BBG_SEND_REQUEST_TIME_OUT);

        sendRequestToMonitorBBG(GET_TEMPERATURE_EVENT_NAME + "\n");
    });

    clientWebSocket.on(GET_DECIBEL_EVENT_NAME, function() {
        eventTimeoutArr[GET_DECIBEL_EVENT_NAME] = setTimeout(function(){
            sendMessageToClient(clientWebSocket, MONITOR_BBG_FAILURE_EVENT_NAME, null);
        }, MONITOR_BBG_SEND_REQUEST_TIME_OUT);

        sendRequestToMonitorBBG(GET_DECIBEL_EVENT_NAME + "\n");
    });

    clientWebSocket.on(GET_TEMPERATURE_DATASET_EVENT_NAME, function() {
        clientWebSocket.emit( GET_TEMPERATURE_DATASET_EVENT_NAME, getDatasetFromFile(TEMPERATURE_DATA_FILE_NAME) );
    });

    clientWebSocket.on(GET_DECIBEL_DATASET_EVENT_NAME, function() {
        clientWebSocket.emit( GET_DECIBEL_DATASET_EVENT_NAME, getDatasetFromFile(SOUND_DATA_FILE_NAME) );
    });
};

function sendRequestToMonitorBBG(data) {
    var buffer = new Buffer(data);

    monitorBBGSocket.send(buffer, 0, buffer.length, MONITOR_BBG_LISTEN_PORT, MONITOR_BBG_LISTEN_ADDR, function(err, bytes) {
        if (err) {
            console.log("[ERROR] failed to send: " + buffer + " to server address: " + MONITOR_BBG_LISTEN_ADDR + " on port: " + MONITOR_BBG_LISTEN_PORT + "\n");
        }
    });
}

function sendRequestToAlarmBBG(data) {
    alarmBBGSocket.write(data);
}

function sendMessageToClient(clientWebSocket, EVENT_NAME, message) {
    clientWebSocket.emit(EVENT_NAME, message);
}

function redirectMonitorBBGResponseToClient(clientWebSocket) {
    // fired when monitor BBG sends back message
    monitorBBGSocket.on('message', function(messageToHost, remote) {
        messageToHost = messageToHost.toString('utf8');
        messageToHost = messageToHost.split(":");
        var eventName = messageToHost[0];
        var messageContent = messageToHost[1];

        clearTimeout(eventTimeoutArr[eventName]);
        clientWebSocket.emit(eventName, messageContent);
    });
}

function redirectAlarmBBGResponseToClient(clientWebSocket) {
    // fired when alarm BBG sends back message
    alarmBBGSocket.on('data', function(messageToHost){
        messageToHost = messageToHost.toString('utf8');
        messageToHost = messageToHost.split(":");

        var eventName = messageToHost[0];
        var messageContent = messageToHost[1];

        clearTimeout(eventTimeoutArr[eventName]);
        clientWebSocket.emit(eventName, messageContent);
    });
}

function getDatasetFromFile(dataFileName){
    var dataFilePath = DATA_FILE_FS + "/" + dataFileName + DATA_FILE_EXTENSION;
    var fileContent;
    var lineContent = "";
    var datasetArr = [];
    var timestampArr = [];
    var formattedTimeStr;
    var datasetObj;
    var dateObj;

    if (prevTemperatureDataFileContent && dataFileName == TEMPERATURE_DATA_FILE_NAME) {
        var startingLineNum = prevTemperatureDataFileContent.length;
    } else if (prevSoundDataFileContent && dataFileName == SOUND_DATA_FILE_NAME) {
        var startingLineNum = prevSoundDataFileContent.length;
    } else {
        var startingLineNum = 0;
    }

    if ( fs.existsSync(dataFilePath) ) {
        fileContent = fs.readFileSync(dataFilePath).toString();

        if (fileContent) {
            // parse file content line-by-line
            for (var i = startingLineNum; i < fileContent.length; i++) {

                if (fileContent[i] != "\n") {
                    lineContent += fileContent[i];
                } else {
                    dateObj = new Date( parseInt(lineContent.split(",")[0]) * 1000 );

                    // timestamp from the data file is UNIX-UTC-timestamp. covert it to ISO format in order to match chart.js requirements
                    formattedTimeStr = dateObj.toISOString();
                    timestampArr.push(formattedTimeStr);
                    datasetArr.push(lineContent.split(",")[1]);
                    lineContent = "";
                }
            }

            datasetObj = {
                timestamp: timestampArr,
                dataset: datasetArr
            }

            if (dataFileName == TEMPERATURE_DATA_FILE_NAME) {
                prevTemperatureDataFileContent = fileContent;
            } else if (dataFileName == SOUND_DATA_FILE_NAME) {
                prevSoundDataFileContent = fileContent;
            }

            return datasetObj;
        }
    }

    return undefined;
}