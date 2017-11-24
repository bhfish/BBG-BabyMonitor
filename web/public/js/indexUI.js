 "use strict"

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
const GET_DECIBEL_DATASET_EVENT_NAME = "getDecibelDataset"
const MONITOR_BBG_FAILURE_EVENT_NAME = "monitorSystemFailure"
const ALARM_BBG_FAILURE_EVENT_NAME = "alarmSystemFailure"

// connection between the web/client and the nodejs server
var nodeServerSocket = io.connect();
var temperatureChartObj = undefined;
var decibelChartObj = undefined;
var isMonitorRunning = false;
var isAlarmRunning = false;

var temperatureCTX = $("#temperatureChart")[0].getContext('2d');
var decibelCTX = $("#decibelChart")[0].getContext('2d');

// client side js begins here
$(document).ready(function() {
    console.log("Document loaded");
    turnOffErrorMessage();
    displayDefaultValues();

    // fired upon disconnection from the nodejs server
    nodeServerSocket.on('disconnect', function(){
        displayErrorMessage(SERVER_ERROR_MESSAGE);
        displayDefaultValues();
        destroyAllGraph();
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
    }, 1000);

    // get the current temperature data for every 15s
    setInterval(function(){
        getBabyRoomTemperatureDataset();
    }, 15000);

    // get the current sound data for every 10s
    setInterval(function(){
        getBabyRoomDecibelDataset();
    }, 10000);

    // callback to turn-off system error message
    setInterval(function(){
        if (isAlarmRunning == true && isMonitorRunning == true) {
            turnOffErrorMessage();
        }
    }, 1000);

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

// send the request for the dataset of baby's room decibel to monitor BBG
function getBabyRoomDecibelDataset() {
    nodeServerSocket.emit(GET_DECIBEL_DATASET_EVENT_NAME, null);
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

    nodeServerSocket.on(GET_DECIBEL_EVENT_NAME, function(messageFromServer) {
        displayDecibel(messageFromServer);
    });

    nodeServerSocket.on(GET_TEMPERATURE_DATASET_EVENT_NAME, function(datasetObj) {
        if (isMonitorRunning == true) {
            if (datasetObj) {
                if (temperatureChartObj) {
                    updateGraph(temperatureChartObj, datasetObj);
                } else {
                    temperatureChartObj = renderGraph(datasetObj, temperatureCTX, "Baby's Room Temperature", "time", "Temperature °C");
                }
            }
        }
    });

    nodeServerSocket.on(GET_DECIBEL_DATASET_EVENT_NAME, function(datasetObj) {
        if (isMonitorRunning == true) {
            if (datasetObj) {
                if (decibelChartObj) {
                    updateGraph(decibelChartObj, datasetObj);
                } else {
                    decibelChartObj = renderGraph(datasetObj, decibelCTX, "Baby's Decibel", "time", "Decibel dB");
                }
            }
        }
    });

    nodeServerSocket.on(MONITOR_BBG_FAILURE_EVENT_NAME, function() {
        isMonitorRunning = false;
        displayMonitorStatus("Inactive");
        displayErrorMessage(SYSTEM_ERROR_MESSAGE);
        displayTemperature("Unknown");
        displayDecibel("Unknown");

        // destroy the graphs on the web if monitor BBG is not running
        destroyAllGraph();
    });

    nodeServerSocket.on(ALARM_BBG_FAILURE_EVENT_NAME, function() {
        isAlarmRunning = false;
        displayAlarmStatus("Inactive");
        displayErrorMessage(SYSTEM_ERROR_MESSAGE);
    });
}

function renderGraph(datasetObj, ctx, graphTitle, xAxisLabel, yAxisLabel){
    var graphObj = undefined;
    var chartConfig = {
        type: 'line',
        data: {
            labels: datasetObj.timestamp,
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
                        labelString: xAxisLabel
                    }
                }],
                yAxes: [{
                    display: true,
                    scaleLabel: {
                        display: true,
                        labelString: yAxisLabel
                    }
                }]
            }
        }
    };

    graphObj = new Chart(ctx, chartConfig);

    return graphObj;
}

// new data to be added to the pre-existed graph and will be displayed dynamically afterwards
function updateGraph(graphObj, datasetObj) {
    for (var i = 0; i < datasetObj.dataset.length; i++) {
        graphObj.data.datasets.forEach((dataset) => {
            dataset.data.push(datasetObj.dataset[i]);
        });

        graphObj.data.labels.push(datasetObj.timestamp[i]);
    }

    graphObj.update();
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
    $('#babyRoomTemperature').text(temperature);

    if (temperature == "Unknown" || temperature.indexOf("Abnormal") >= 0 ) {
        $('#babyRoomTemperature').css({'color': 'red', 'font-weight': 'bold'});
    } else {
        $('#babyRoomTemperature').css({'color': 'blue', 'font-weight': 'bold'});
    }
}

function displayDecibel(decibel) {
    $('#decibelLevel').text(decibel);

    if (decibel == "Unknown" || decibel.indexOf("Abnormal") >= 0) {
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

function destroyAllGraph() {
    if (temperatureChartObj) {
        temperatureChartObj.destroy();
        temperatureChartObj = undefined;
    }

    if (decibelChartObj) {
        decibelChartObj.destroy();
        decibelChartObj = undefined;
    }
}