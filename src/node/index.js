import "./index.html"
import "mini.css/dist/mini-default.css"
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
  show("home");
  reload();
});
