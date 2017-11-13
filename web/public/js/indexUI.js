 "use strict"

/*
    TODO:
    2) Add decibel display on web
    3) fix the the size of the graph when updating
*/
const SERVER_ERROR_MESSAGE = "Internal Server Error: server is not running." +
                                "The server encountered an internal error or misconfiguration and was unable to complete your request." +
                                "Please contact the Third Eye maintenance team: thirdEye-maintain@thirdEye.com";
const SYSTEM_ERROR_MESSAGE = "Internal System Error: system is not running" +
                                "The system encountered an internal error and further monitoring services may be impacted." +
                                "Please reach out your baby ASAP and contact the Third Eye maintenance team: thirdEye-maintain@thirdEye.com later";

// different socket events
const GET_MONITOR_BBG_STATUS_EVENT_NAME = "getMonitorBBGStatus";
const GET_ALARM_BBG_STATUS_EVENT_NAME = "getAlarmBBGStatus";
const GET_TEMPERATURE_EVENT_NAME = "getTemperature";
const GET_DECIBEL_EVENT_NAME = "getDecibel"
const GET_TEMPERATURE_DATASET_EVENT_NAME = "getTemperatureDataset"
const MONITOR_BBG_FAILURE_EVENT_NAME = "monitorSystemFailure"
const ALARM_BBG_FAILURE_EVENT_NAME = "alarmSystemFailure"

// connection between the web/client and the nodejs server
var nodeServerSocket = io.connect();
var temperatureChartObj = undefined;
var isMonitorRunning = false;
var isAlarmRunning = false;

// client side js begins here
$(document).ready(function() {
    console.log("Document loaded");
    turnOffErrorMessage();
    displayDefaultValues();

    // fired upon disconnection from the nodejs server
    nodeServerSocket.on('disconnect', function(){
        displayErrorMessage(SERVER_ERROR_MESSAGE);
        displayDefaultValues();

        if (temperatureChartObj) {
            temperatureChartObj.destroy();
        }
    });

    // get the monitor BBG and alarm BBG running status for every 1s
    setInterval(function(){
        getMonitorBBGRunningStatus();
    }, 1000);

    setInterval(function(){
        getAlarmBBGRunningStatus();
    }, 1000);

    // get the current baby's room temperature for every 1s
    setInterval(function(){
        getBabyRoomTemperature();
    }, 1000);

    // get the current decibel from baby's room for every 1s
    setInterval(function(){
        getDecibel();
    });

    // get the current temperature data for every 10s
    setInterval(function(){
        getBabyRoomTemperatureDataset();
    }, 10000);

    // callback to turn-off system error message
    // setInterval(function(){
    //     if (isAlarmRunning == true && isMonitorRunning == true) {
    //         turnOffErrorMessage();
    //     }
    // }, 1000);

    // listen on any response/specified event from the server
    waitForNodeServerResponse();
});

// send the request for the running status of the monitor BBG
function getMonitorBBGRunningStatus() {
    nodeServerSocket.emit(GET_MONITOR_BBG_STATUS_EVENT_NAME, null);
}

// send the request for the running status of the alarm BBG
function getAlarmBBGRunningStatus() {
    nodeServerSocket.emit(GET_ALARM_BBG_STATUS_EVENT_NAME, null);
}

// send the request for the current baby's room temperature to monitor BBG
function getBabyRoomTemperature() {
    nodeServerSocket.emit(GET_TEMPERATURE_EVENT_NAME, null);
}

// send the request for current baby's room temperature to monitor BBG
function getDecibel(){
    nodeServerSocket.emit(GET_DECIBEL_EVENT_NAME, null);
}

// send the request for the dataset of baby's room temperature to monitor BBG
function getBabyRoomTemperatureDataset() {
    nodeServerSocket.emit(GET_TEMPERATURE_DATASET_EVENT_NAME, null);
}

function waitForNodeServerResponse() {

    nodeServerSocket.on(GET_MONITOR_BBG_STATUS_EVENT_NAME, function(messageFromServer) {
        displayMonitorStatus(messageFromServer);
    });

    nodeServerSocket.on(GET_ALARM_BBG_STATUS_EVENT_NAME, function(messageFromServer) {
        displayAlarmStatus(messageFromServer);
    });

    nodeServerSocket.on(GET_TEMPERATURE_EVENT_NAME, function(messageFromServer) {
        displayTemperature(messageFromServer);
    });

    nodeServerSocket.on(GET_TEMPERATURE_DATASET_EVENT_NAME, function(datasetObj) {
        // if (isAlarmRunning == true && isMonitorRunning == true) {
            if (datasetObj) {
                renderGraph( datasetObj, $("#temperatureChart")[0].getContext('2d'), "Baby's Room Temperature", "Temperature °C");
            } else {
                if (temperatureChartObj) {
                    temperatureChartObj.destroy();
                }
            }
        // }
    });

    nodeServerSocket.on(MONITOR_BBG_FAILURE_EVENT_NAME, function() {
        isMonitorRunning = false;
        displayErrorMessage(SYSTEM_ERROR_MESSAGE);
    });

    // nodeServerSocket.on(ALARM_BBG_FAILURE_EVENT_NAME, function() {
    //     isAlarmRunning = false;
    //     displayErrorMessage(SYSTEM_ERROR_MESSAGE);
    // });
}

function renderGraph(datasetObj, ctx, graphTitle, yAxesLabel){
    var chartConfig = {
        type: 'line',
        data: {
            labels:datasetObj.timestamp,
            datasets: [{
                label: graphTitle,
                data: datasetObj.dataset,
                fill: false,
                lineTension: 0,
                borderWidth: 2,

                // data point color
                backgroundColor: "red",

                // line color
                borderColor: "red"
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                xAxes: [{
                    type: "time",
                    distribution: 'series',
                    display: true,
                    scaleLabel: {
                        display: true,
                        labelString: 'Time'
                    }
                }],
                yAxes: [{
                    display: true,
                    scaleLabel: {
                        display: true,
                        labelString: yAxesLabel
                    }
                }]
            }
        }
    };

    temperatureChartObj = new Chart(ctx, chartConfig);
}

function displayErrorMessage(message) {
    $('#error-text').text(message);
    $('#error-box').show();
}

function turnOffErrorMessage() {
    $('#error-box').hide();
}

function displayMonitorStatus(monitorStatus) {
    $('#monitorStatus').text(monitorStatus);

    if (monitorStatus == "Active") {
        isMonitorRunning = true;
        $('#monitorStatus').css({'color': 'green', 'font-weight': 'bold'});
    } else {
        isMonitorRunning = false;
        $('#monitorStatus').css({'color': 'red', 'font-weight': 'bold'});
    }
}

function displayAlarmStatus(alarmStatus) {
    $('#alarmStatus').text(alarmStatus);

    if (alarmStatus == "Active") {
        isAlarmRunning = true;
        $('#alarmStatus').css({'color': 'green', 'font-weight': 'bold'});
    } else {
        isAlarmRunning = false;
        $('#alarmStatus').css({'color': 'red', 'font-weight': 'bold'});
    }
}

function displayTemperature(temperature) {
    $('#babyRoomTemperature').text(temperature + " °C");

    if (temperature == "Unknown") {
        $('#babyRoomTemperature').css({'color': 'red', 'font-weight': 'bold'});
    } else {
        $('#babyRoomTemperature').css({'color': 'blue', 'font-weight': 'bold'});
    }
}

function displayDecibel(decibel) {
    $('#decibelLevel').text(decibel + " dB");

    if (decibel == "Unknown") {
        $('#decibelLevel').css({'color': 'red', 'font-weight': 'bold'});
    } else {
        $('#decibelLevel').css({'color': 'blue', 'font-weight': 'bold'});
    }
}

function displayDefaultValues() {
    displayMonitorStatus("Inactive");
    displayAlarmStatus("Inactive");
    displayTemperature("Unknown");
    displayDecibel("Unknown");
}