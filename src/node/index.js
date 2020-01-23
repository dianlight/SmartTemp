import "./index.html"
import "mini.css/dist/mini-default.css"
import "./holds.css"
import "./favicon.ico"

import "chibijs"
import populate from "populate.js";

const CPROG = ['eco','normal','confort'];

function reload(){
    $().ajax("load","GET",function(data,status){
        console.log(status,data);
        let jdata = JSON.parse(data);
        populate($("#configForm")[0],jdata);
    });
}
window.reload = reload;

function reloadProgram(){
  $().ajax("loadP","GET",function(data,status){
      console.log(status,data);
      let jdata = JSON.parse(data);
      ['mon','tue','wed','thu','fri','sat','sun'].forEach(function(d,index,allarray){
        $("#"+d).html("");
        for(let h=0,hq=0; h < 24; h++,hq+=4){
          $("#"+d)
            .htmlAppend("<sup>"+h+"</sup>")
            .htmlAppend("<div id='C"+index+"_"+hq+"' onclick='change("+index+","+hq+");' class=\""+CPROG[jdata.wprg[index].prg[hq]]+"\"></div>")
            .htmlAppend("<div id='C"+index+"_"+(hq+1)+"' onclick='change("+index+","+(hq+1)+");' class=\""+CPROG[jdata.wprg[index].prg[hq+1]]+"\"></div>")
            .htmlAppend("<div id='C"+index+"_"+(hq+2)+"' onclick='change("+index+","+(hq+2)+");' class=\""+CPROG[jdata.wprg[index].prg[hq+2]]+"\"></div>")
            .htmlAppend("<div id='C"+index+"_"+(hq+3)+"' onclick='change("+index+","+(hq+3)+");' class=\""+CPROG[jdata.wprg[index].prg[hq+3]]+"\"></div>");
        }
      });
  });
}
window.reload = reloadProgram;

function save(){

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

  $("#configForm").ajax("save","POST",function(data,status){
    reload();
  });

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

function show(what){
  $(".panel").hide();
  $("#"+what).show();
}
window.show = show;

$().ready(function(){

  // Home

  // Program
  reloadProgram();

  // Config
  reload();

  // StartPage
  show("home");
  

});
