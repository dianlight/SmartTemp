import "mini.css/dist/mini-default.css"
import "./holds.css"
import "./favicon.ico"

import "xterm/css/xterm.css"
import { Terminal } from 'xterm';
import { FitAddon } from 'xterm-addon-fit';
import ReconnectingWebSocket from 'reconnecting-websocket';

import "chibijs"
import populate from "populate.js"

const CPROG = ['eco','normal','confort'];
const DAY_NAMES = ['mon','tue','wed','thu','fri','sat','sun'];

let displayTimeout;
let currentPanel = "home";

function reload(){
    $().ajax("load","GET",function(data,status){
        console.log(status,data);
        let jdata = JSON.parse(data);
        populate($("#configForm")[0],jdata);
        $("#saveButtons").show();
    });
}
window.reload = reload;


function reloadProgram(index = 0){
      if(index == 7){
        $("#saveButtons").show();
      } else {
        $().ajax("loadP?day="+index,"GET",function(data,status){
          console.log(status,data);
          let jdata = JSON.parse(data);
          $("#"+DAY_NAMES[index]).html("");
          for(let h=0,hq=0; h < 24; h++,hq+=4){
            $("#"+DAY_NAMES[index])
              .htmlAppend("<sup>"+h+"</sup>")
              .htmlAppend("<div id='C"+index+"_"+hq+"' onclick='change("+index+","+hq+");' class=\""+CPROG[jdata[hq]]+"\"></div>")
              .htmlAppend("<div id='C"+index+"_"+(hq+1)+"' onclick='change("+index+","+(hq+1)+");' class=\""+CPROG[jdata[hq+1]]+"\"></div>")
              .htmlAppend("<div id='C"+index+"_"+(hq+2)+"' onclick='change("+index+","+(hq+2)+");' class=\""+CPROG[jdata[hq+2]]+"\"></div>")
              .htmlAppend("<div id='C"+index+"_"+(hq+3)+"' onclick='change("+index+","+(hq+3)+");' class=\""+CPROG[jdata[hq+3]]+"\"></div>");
          }
          reloadProgram(index+1);
        });
      }
}
window.reload = reloadProgram;

function saveConfig(){
  $("#configForm").ajax("save","POST",function(data,status){
    reload();
  });
}

function saveProgram(){
  ['mon','tue','wed','thu','fri','sat','sun'].forEach(function(d,index,allarray){
    let json = new Array();
    for(let hq=0; hq < 24*4; hq++){
      json[hq]=CPROG.indexOf($("#C"+index+"_"+hq+"").removeClass("unsaved").getClass());
    }
    $("#d"+d).val(JSON.stringify(json));
  });

  $("#programForm").ajax("saveP","POST",function(data,status){
    reloadProgram();
  });
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
  let cls = $("#C"+day+"_"+hq).getClass();
  let p = CPROG.indexOf(cls);
  console.log("Letto",cls,p,CPROG);
  p = (p+1) % 3;
  console.log(p, CPROG[p]);
  $("#C"+day+"_"+hq).setClass(CPROG[p]+" unsaved");
}

window.change = change;

const term = new Terminal();

function show(what){
  $(".panel").hide();
  $('header a').removeClass('bordered');
  $("#saveButtons").hide();
  currentPanel=what;
  if(currentPanel == "home"){
    refreshDisplayFromMap();
  }else if(currentPanel == "program"){
    clearTimeout(displayTimeout);
    reloadProgram();
  } else if(currentPanel == "config"){
    clearTimeout(displayTimeout);
    reload();
  } else if(currentPanel == "debug"){
    clearTimeout(displayTimeout);
    term.scrollToBottom();
  }
  $("#"+what+"Tab").toggleClass("bordered");
  $("#"+what).show();
}
window.show = show;

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

//  show("home");
  show("debug");

  // WebSockets
  var debug_service = new ReconnectingWebSocket('ws://'+window.location.host+'/ws');
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
});
