
var sendTxt = "";
var dropped;
var max = 15;
var DID;  // Dragged ID
var DIH;  // Dragged innerHTML
var LDO;  // Last Dragged Over ID
var LDH;  // Last Dragged Over innerHTML

function showTurnouts() {
  max = 15;
  document.getElementById("cHide").setAttribute("hidden","false");
  document.getElementById("tHide").removeAttribute("hidden");
  document.getElementById("btn-TOs").disabled = true;
  document.getElementById("btn-cabs").disabled = false;
}

function showCabs() {
  max = 10;
  document.getElementById("tHide").setAttribute("hidden", "false");
  document.getElementById("cHide").removeAttribute("hidden");
  document.getElementById("btn-cabs").disabled = true;
  document.getElementById("btn-TOs").disabled = false;
}

var tElement = document.getElementById("turnouts");
var tRows = tElement.querySelectorAll(".drag");
var cElement = document.getElementById("cabs");
var cRows = cElement.querySelectorAll(".drag");
tRows.forEach(addEventListeners);
cRows.forEach(addEventListeners);

function addEventListeners(item) {
  item.setAttribute("draggable","true");
  item.addEventListener("click", function () {
    editItem(item.id);
  });
  item.addEventListener("dragstart", dragStart);
  item.addEventListener("dragend", dragEnd);
  item.addEventListener("dragover", dragOver);
  item.addEventListener("dragenter", dragEnter);
  item.addEventListener("dragleave", dragLeave);
  item.addEventListener("drop", drop);
}

function editItem(tID) {
  modal.style.display = "block";
  DID = tID;
  sendTxt = "<I "+DID+">";
  connection.send(sendTxt);
}



function dragStart( event ) {
  var t = event.target;
  event.dataTransfer.setData("text", t.innerHTML);
  DID = LDO = t.id;
  DIH = t.innerHTML;
  t.style.opacity = .5;
  dropped = false;
  event.stopPropagation();
}

function dragEnd( event ) {
  var t = event.target;
  if ( t.className = "drag") {
    t.style.opacity = "";
    // Reset innerHTML if dragged out of bounds
    if (dropped == false) {
      var DIDN = Number(DID.substr(1));
      var LDON = Number(LDO.substr(1));
      if (max>10) {
        resetRows(tRows);
      } else {
        resetRows(cRows);
      }
      function resetRows (rows) {
        if (LDON>0) {  // shift up
          for (x=max-1; x>DIDN-1; x--) {
          rows[x].innerHTML = rows[x-1].innerHTML;
          rows[x].color = rows[x-1].color;
          }
        } else {  // shift down
          for (x=0; x<DIDN; x++) {
            rows[x].innerHTML = rows[x+1].innerHTML;
            rows[x].color = rows[x+1].color;
          }
        }
        rows[DIDN].innerHTML = DIH;
      }
    } else {
      sendTxt = "<I "+DID+" "+LDO+">";
      connection.send(sendTxt);
    }
  }
  event.stopPropagation();
}

/* events fired on the drop targets */
function dragEnter( event ) {
    // highlight potential drop target when the draggable element enters it
    var t = event.target;
    if ( t.className = "drag") {
        t.style.background = "purple";
        // document.getElementById(LDO).innerHTML = t.innerHTML;
        LDH = t.innerHTML;
        t.innerHTML = "- - -";
      }
      event.stopPropagation();
}

function dragOver( event ) {
  // prevent default to allow drop
  event.preventDefault();
  LDO = event.target.id;
  event.stopPropagation();
}

function dragLeave( event ) {
    // reset background of potential drop target when the draggable element leaves it
  var t = event.target;
  if ( t.className == "drag" ) {
    t.style.background = "";
    t.innerHTML = LDH;
  }
  event.stopPropagation();
}

function drop( event ) {
    // prevent default action (open as link for some elements)
  event.preventDefault();
  var t = event.target;
  if ( t.className == "drag" ) {
    t.style.background = "";
    t.innerHTML = event.dataTransfer.getData("text");
    dropped = true;
  }
  event.stopPropagation();
}

// Modal section
var modal = document.getElementById("myModal");

function saveEdit() {
  var icon = document.getElementById("lIcon").value;
  var pin = document.getElementById("pin").value;
  var checked = document.getElementById("showBtn").checked;
  var show = checked ? 0 : 1;
  var title = document.getElementById("title").value;
  sendTxt = "<K "+DID+" "+show+" "+pin+" "+icon+" "+title+">";
  connection.send(sendTxt);
  modal.style.display = "none";
}

window.onclick = function(event) {
  if (event.target == modal) {
    modal.style.display = "none";
  }
}


// Communication

var connection = new WebSocket("ws://" + location.hostname + ":81/", ['arduino']);

function onMessage(event) {
  var messageTxt = event.data;
  var last = messageTxt.lastIndexOf(">");
  var com = messageTxt.substr(1,last-1);
  var com1 = com.substr(0,1);
  var pin = com.substr(8,3);
  if (com1 == "I") {
    // DID = com.substr(2,3);
    if (modal.style.display === "block") {
      document.getElementById("showBtn").checked = Number(com.substr(6,1));
      document.getElementById("pin").value = pin;
      document.getElementById("lIcon").value = com.substr(12,2);
      document.getElementById("title").value = com.substr(15);
    } else {
      var tID = com.substr(2,3);
      var element = document.getElementById(tID);
      var show = Number(com.substr(6,1));
      if (show<1) {
        pin = "👁️  " + pin + "  ";
      } else {
        pin = "💤  " + pin + "  ";
      }
      element.innerHTML = pin + com.substr(12);
    }
  }
}

connection.onopen = function () {
  connection.send('Connect ' + new Date());
  tRows.forEach(getLine);
  cRows.forEach(getLine);
  function getLine(item) {
    setTimeout(sendM, 100);
    function sendM () {
      sentTxt = "<I "+item.id+">";
      connection.send(sentTxt);
    }
  }
}

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
}

connection.onmessage = function (event) {
  console.log('Server: ', event.data);
  onMessage(event);

}

connection.onclose = function () {
  console.log('WebSocket connection closed');
}
