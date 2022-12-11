let temp_level = "30"
let humu_level = "50"

var gateway = `ws://${httpGet("http://esp8266.local")}/ws`;
// var gateway = "ws://192.168.10.107/ws"
console.log("gateway ----> " + gateway );
var websocket;

function httpGet(theUrl)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", theUrl, false ); 
    xmlHttp.send( null );
    return xmlHttp.responseText;
}

window.addEventListener('load', onload);

function onload(event) {
    initWebSocket();
}

function getValues(){
    websocket.send("getValues");
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValues();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function updateStatusSW(element) {
  var swNumber = element.id.charAt(element.id.length-1);
  if ($(`#togBtn_${swNumber}`).is(':checked')) {
    websocket.send(swNumber+"s"+100);
  }
  else {
    websocket.send(swNumber+"s"+0);
  }
}

function onMessage(event) {
    var myObj = JSON.parse(event.data);
    var keys = Object.keys(myObj);
    for (var i = 0; i < keys.length; i++){
        var key = keys[i];
        if (key.includes("sliderValue")){
          if (myObj[key] == "100") {
            $(`#togBtn_${(i+1).toString()}`).prop('checked', true);
          } else {
            $(`#togBtn_${(i+1).toString()}`).prop('checked', false);
          }
        }
        if (key.includes("DHT_")){
            console.log("in this DHT case");
            $("#DHT_Humidity").text(myObj["DHT_Humidity"]);
            $("#DHT_Temperature").text(myObj["DHT_Temperature"]);

            if (myObj["DHT_Temperature"] == temp_level){
              $(`#togBtn_3.toString()}`).prop('checked', true);
            }
            if (myObj["DHT_Temperature"] == humu_level){
              $(`#togBtn_4.toString()}`).prop('checked', true);
            }
        }
    }
}

var intervalId = window.setInterval(function(){
  // console.log("test setInterval");
  websocket.send("DHT");
}, 5000);
