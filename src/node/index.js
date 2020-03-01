import "mini.css/dist/mini-default.css"
import "./holds.css"
import "./favicon.ico"

import "xterm/css/xterm.css"
import { Terminal } from 'xterm';
import { FitAddon } from 'xterm-addon-fit';
import ReconnectingWebSocket from 'reconnecting-websocket';

import $ from "cash-dom";
import populate from "populate.js"

const CPROG = ['eco','normal','confort'];
const DAY_NAMES = ['mon','tue','wed','thu','fri','sat','sun'];

let displayTimeout;
let currentPanel = "home";

function reload(){
  fetch("load")
    .then(response => response.json())
    .then(jdata => {
      populate($("#configForm")[0],jdata);
      $("#saveButtons").show();
    })
    .catch(err => console.error(err));
}
window.reload = reload;

function reloadWifi(){
  fetch("loadW")
    .then(response => response.json())
    .then(jdata => {
      populate($("#wifiConfigForm")[0],jdata);
      $("#connectButtons").show();
  })
  .catch(err => console.error(err));
}

function scanWifi(){
  $("#wifiScanButton").hide();
  $("#wifiScanResult").hide();
  $("#wifiScanTable").html("");
  $("#wifiScanSpinner").show();
  fetch("scanW")
  .then(response => response.json())  
  .then(jdata => {
    console.log(jdata);
  })
  .catch(err => console.error(err));
}

window.scanWifi = scanWifi;

function validateWifi(){
//  console.log("Validate! ",$("#ssid").val() !== "" ,$("#wkey").val() !== "");
  if( $("#ssid").val() !== "" && $("#wkey").val() !== ""){
    $("#wifiConnectButton").removeAttr("disabled");
  } else {
    $("#wifiConnectButton").attr("disabled","disabled");
  }
}
window.validateWifi = validateWifi;

function reloadProgram(index = 0){
      if(index == 7){
        $("#saveButtons").show();
      } else {
        fetch("loadP?day="+index)
        .then(response => response.json())  
        .then(jdata => {
          console.log(jdata);
          $("#"+DAY_NAMES[index]).html("");
          for(let h=0,hq=0; h < 24; h++,hq+=4){
            $("#"+DAY_NAMES[index])
              .append("<sup>"+h+"</sup>")
              .append("<div id='C"+index+"_"+hq+"' onclick='change("+index+","+hq+");' class=\""+CPROG[jdata[hq]]+"\"></div>")
              .append("<div id='C"+index+"_"+(hq+1)+"' onclick='change("+index+","+(hq+1)+");' class=\""+CPROG[jdata[hq+1]]+"\"></div>")
              .append("<div id='C"+index+"_"+(hq+2)+"' onclick='change("+index+","+(hq+2)+");' class=\""+CPROG[jdata[hq+2]]+"\"></div>")
              .append("<div id='C"+index+"_"+(hq+3)+"' onclick='change("+index+","+(hq+3)+");' class=\""+CPROG[jdata[hq+3]]+"\"></div>");
          }
          reloadProgram(index+1);
        })
        .catch(err => console.error(err));
      }
}
window.reload = reloadProgram;

async function saveConfig(){
  console.log($("#configForm")[0]);
  const data = new URLSearchParams(new FormData($("#configForm")[0]));
  console.log(data);
  await fetch("save",{
    method: 'POST',
    body: data
  });
  reload();
}

async function saveProgram(){
  ['mon','tue','wed','thu','fri','sat','sun'].forEach(function(d,index,allarray){
    let json = new Array();
    for(let hq=0; hq < 24*4; hq++){
      json[hq]=CPROG.indexOf($("#C"+index+"_"+hq+"").removeClass("unsaved").attr("class"));
    }
    $("#d"+d).val(JSON.stringify(json));
  });

  const data = new URLSearchParams(new FormData($("#programForm")[0]));
  await fetch("saveP",{
    method: 'POST',
    body: data
  });

  reloadProgram();
}

function save(){
  if(currentPanel == "home"){
    // No save action
  }else if(currentPanel == "program"){
    saveProgram();
  } else if(currentPanel == "config"){
    saveConfig();
  }
}
window.save = save;

function change(day,hq){
  $("#C"+day+"_"+hq).removeClass("unsaved");
  let cls = $("#C"+day+"_"+hq).attr("class");
  let p = CPROG.indexOf(cls);
  console.log("Letto",cls,p,CPROG);
  p = (p+1) % 3;
  console.log(p, CPROG[p]);
  $("#C"+day+"_"+hq).attr("class",CPROG[p]+" unsaved");
}

window.change = change;

const term = new Terminal();

function show(what){
  $(".panel").hide();
  $('header a').removeClass('bordered');
  $("#saveButtons").hide();
  $("#connectButtons").hide();
  currentPanel=what;
  if(currentPanel == "home"){
    refreshDisplayFromMap();
  }else if(currentPanel == "program"){
    clearTimeout(displayTimeout);
    reloadProgram();
  } else if(currentPanel == "config"){
    clearTimeout(displayTimeout);
    reload();
  } else if(currentPanel == "wificonfig"){
    clearTimeout(displayTimeout);
    reloadWifi();
  } else if(currentPanel == "debug"){
    clearTimeout(displayTimeout);
    term.scrollToBottom();
  }
  $("#"+what+"Tab").toggleClass("bordered");
  $("#"+what).show();
}
window.show = show;

async function wps(){
  console.log("WPS!");
  await fetch("wps");
  location.reload(true);
}
window.wps = wps;

async function connect(){
  console.log($("#wifiConfigForm")[0]);
  const data = new URLSearchParams(new FormData($("#wifiConfigForm")[0]));
  console.log(data);
  await fetch("saveW",{
    method: 'POST',
    body: data
  });
  location.reload(true);
}
window.connect = connect;

function refreshDisplayFromMap(){
  var oReq = new XMLHttpRequest();
  oReq.open("GET", "screenpbm", true);
  oReq.responseType = "arraybuffer";
  
  oReq.onload = function (oEvent) {
    var BreakException = {};
    var arrayBuffer = oReq.response; // Note: not oReq.responseText
    if (arrayBuffer) {
      var byteArray = new Uint8Array(arrayBuffer);
      if(byteArray[0] != 0x50 || byteArray[1] != 0x34){
        console.error("Only BPM P4 (Binary) are supported!")
        throw BreakException;
      }
      let x_size =0,y_size=0;
      let ascii_text="";      
      for (var i = 3; i < byteArray.byteLength; i++) {
        if(x_size == 0 && byteArray[i] == 0x0A){
          x_size = parseInt(ascii_text);
          ascii_text = "";
        } else if(y_size == 0 && byteArray[i] == 0x20){
          y_size = parseInt(ascii_text);
        } else if(x_size == 0 || y_size == 0){
          ascii_text=ascii_text+String.fromCharCode(byteArray[i]);
        } else {
          byteArray = byteArray.subarray(i);
          break;
        }
      }
      console.log("BPM Size "+x_size+"x"+y_size);

      let ctx = $("#screen")[0].getContext('2d');
      ctx.fillStyle = 'rgb(0, 0, 0)';
      let pos = 0;
      for(let y=0; y < y_size; y++){
        for(let x=0; x < x_size; x+=8){
          for (var i = 7; i >= 0; i--) {
            var bit = (byteArray[pos] & (1 << i)) > 0 ? 1 : 0;
            if(bit == 1)ctx.fillStyle = 'rgb(0, 0, 0)';
            else ctx.fillStyle = 'rgb(255, 255, 255)';
            ctx.fillRect(x+(7-i),y,1,1);
          }
          pos++;
        }
      }
      displayTimeout = setTimeout(refreshDisplayFromMap,500);
    }
  };
  oReq.send(null);
}

$().ready(function(){

  // Debug
  const fitAddon = new FitAddon();
  term.loadAddon(fitAddon);
  term.open($("#terminal")[0]);
  fitAddon.fit();
  term.writeln('Console \x1B[1;3;31mconnecting...\x1B[0m');

  // StartPage
  fetch("ostat")
    .then(response => response.json())
    .then(data => {
      console.log(data,"First tab",data.otab);
      show(data.otab);
      if(data.bdg){
        alert(data.bdg);
      }
    })
    .catch( err => console.error(err));
  
  // WebSockets
  var debug_service = new ReconnectingWebSocket('ws://'+window.location.host+'/log');
  debug_service.onmessage = function(event){
   // console.log("Messaggio",event);
    term.writeln(event.data);
  }
  debug_service.onopen = function(event){
   // console.log(event);
   term.writeln('\x1B[1;3;31mConnected\x1B[0m');
   debug_service.send("CSL");
  }
  debug_service.onclose = function(){
    term.writeln('\x1B[1;3;31mDisconnected\x1B[0m');
  }
  debug_service.onerror = function(){
    term.writeln('\x1B[1;3;31mError!\x1B[0m');
  }
  

  var scan_service = new ReconnectingWebSocket('ws://'+window.location.host+'/scan');
  scan_service.onmessage = function(event){
    let item = JSON.parse(event.data);
    console.log("Got Scan Result",event.data);
    $("#wifiScanSpinner").hide();
    if(item.end){
      $("#wifiScanButton").show();
      validateWifi();
    } else {
      // Search networks
//      console.log($("#wifiScanTable tr td[data-ssid='"+item.ssid+"'][data-label='SSID']"));
      if($("#wifiScanTable tr td[data-ssid='"+item.ssid+"'][data-label='SSID']").length != 0){
        console.log("Duplicate Network",item);
        let str = $("#wifiScanTable tr td[data-ssid='"+item.ssid+"'][data-label='Strength']");
        str.text(str.text()+"/"+item.dBm+"dBm");
      } else {
        let html = "<tr id=\"netw"+item.i+"\">";
        html+="<td data-label=\"SSID\" data-ssid=\""+item.ssid+"\">"+item.ssid+"</td>";
        html+="<td data-Label=\"Strength\" data-ssid=\""+item.ssid+"\">"+item.dBm+"dBm</td>";
        html+="<td data-label=\"Type\" data-ssid=\""+item.ssid+"\">"+(item.open?"":"<span class=\"icon-lock\"></span>")+"</td>";
        html+="</tr>";
        $("#wifiScanTable").append(html);
        $("#netw"+item.i).on( 'click', event => {
          console.log(item.ssid);
          $("#ssid").val(item.ssid)[0].scrollIntoView();
          $("#wkey")[0].focus();
        });
        $("#wifiScanResult").show();
        }
    }
  }
  scan_service.onopen = function(event){
    console.log("Connect WS /scan",event);
    scan_service.send("CSL2");
  }
  scan_service.onclose = function(){
    console.log("Disconect WS /scan");
  }
  scan_service.onerror = function(){
    console.log("Error WS /scan");
  }
   
});
