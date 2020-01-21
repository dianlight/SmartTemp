import "./index.html"
import "mini.css/dist/mini-default.css"
import "./holds.css"
import "./favicon.ico"

import "chibijs"
import populate from "populate.js";

function reload(){
    $().ajax("load","GET",function(data,status){
        console.log(status,data);
        populate($("#configForm")[0],JSON.parse(data));
    });

}
window.reload = reload;

function save(){
  $("#configForm").ajax("save","POST",function(data,status){
      reload();
  });
}
window.save = save;

function show(what){
  $(".panel").hide();
  $("#"+what).show();
}
window.show = show;

$().ready(function(){

  // Home

  // Program
  for(let d of ['mon','tue','wed','thu','fri','sat','sun']){
    for(let h=0; h < 24; h++){
      $("#"+d)
        .htmlAppend("<sup>"+h+"<sup>")
        .htmlAppend("<div class=\"eco\"></div>")
        .htmlAppend("<div class=\"normal\"></div>")
        .htmlAppend("<div class=\"eco\"></div>")
        .htmlAppend("<div class=\"confort\"></div>");
    }
  }

  // Config
  reload();

  // StartPage
  show("home");
  

});
