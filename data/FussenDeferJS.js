
var tRows = document.getElementById("turns").children;
var cRows = document.getElementById("myDropdown").children;
var qRows = document.getElementById("Q").children;
var pRows = document.getElementById("pSounds").children;

window.onload = function() {
  for (var item in tRows) {
    // item.addEventListener("click", selectLoco);
    getInner(item);
  }
}


/*
cRows.forEach(function(item){
  item.addEventListener("click", setZ);
  getInner(item);
});

function getInner(item) {
  var tID = item.id;
  sendText = "<Z " + tID + ">";
  connection.send(sendText);
}

qRows.forEach(function(item){
  item.addEventListener("click", setZ);
});

pRows.forEach(function(item){
  item.addEventListener("click", setZ);
});*/
