
var sendTxt = "";

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
  item.addEventListener("click", editItem);
  item.addEventListener("dragstart", dragStart);
  item.addEventListener("dragend", dragEnd);
  item.addEventListener("dragover", dragOver);
  item.addEventListener("dragenter", dragEnter);
  item.addEventListener("dragleave", dragLeave);
  item.addEventListener("drop", drop);
}

var dropped;
var max = 15;
var DID;  // Dragged ID
var DIH;  // Dragged innerHTML
var LDO;  // Last Dragged Over ID

  /* events fired on the draggable target */
  //function drag( event ) {

  //};

  function dragStart( event ) {
    event.dataTransfer.setData("text", event.target.innerHTML);
    DID = LDO = event.target.id;
    DIH = event.target.innerHTML;
    event.target.style.opacity = .5;
    dropped = false;
    event.stopPropagation();
  };

  function dragEnd( event ) {
    if ( event.target.className = "drag") {
      event.target.style.opacity = "";
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
            }
          } else {  // shift down
            for (x=0; x<DIDN; x++) {
              rows[x].innerHTML = rows[x+1].innerHTML;
            }
          }
          rows[DIDN].innerHTML = DIH;
        }
      } else {
        var sourceTxt = event.dataTransfer.getData("text");
        var targetTxt = event.target.id;
        sendTxt = "<I "+sourceTxt+" "+targetTxt+">";
        connection.send(sendTxt);
      }
    }
    event.stopPropagation();
  };

  /* events fired on the drop targets */
  function dragOver( event ) {
    // prevent default to allow drop
    event.preventDefault();
    LDO = event.target.id;
    event.stopPropagation();
  };

  function dragEnter( event ) {
      // highlight potential drop target when the draggable element enters it
      var t = event.target;
      if ( t.className = "drag") {
          t.style.background = "purple";
          document.getElementById(LDO).innerHTML = t.innerHTML;
          t.innerHTML = "-";
        }
        event.stopPropagation();
  };

  function dragLeave( event ) {
      // reset background of potential drop target when the draggable element leaves it
    if ( event.target.className == "drag" ) {
      event.target.style.background = "";
    }
    event.stopPropagation();
  };

  function drop( event ) {
      // prevent default action (open as link for some elements)
    event.preventDefault();
    var t = event.target;
    if ( t.className == "drag" ) {
      t.style.background = "";
      t.innerHTML=event.dataTransfer.getData("text");
      dropped = true;
    }
    event.stopPropagation();
  };

  function editItem() {
    DID = this.id;
    sendTxt = "<I "+DID+">";
    connection.send(sendTxt);
  }

  // Modal section
  var test;
  var modal = document.getElementById("myModal");
  var editItemID;

  function saveEdit() {
    var icon = getElementById("icon").value;
    var pin = getElementById("pin").value;
    var hide = getElementById("hideBtn").value;
    var title = getElementById("title").value;
    sendTxt = "<I "+DID+" "+icon+" "+pin+" "+hide+" "+"title"+">";
    connection.send(sendTxt);
    modal.style.display = "block";
  }

  window.onclick = function(event) {
    if (event.target == modal) {
      modal.style.display = "none";
    }
  }

/*
  // Communication

  var connection = new WebSocket("ws://" + location.hostname + ":81/", ['arduino']);

  function onMessage(event) {
    var messageTxt = event.data;
    var last = messageTxt.lastIndexOf(">");
    var com = messageTxt.substr(1,last-1);
    var com1 = com.substr(0,1);
    if (com1 == "I") {
      DID = com.substr(2,5);
      if (modal.display === "none") {
        var element = document.getElementById(DID);
        element.hidden = com.substr(6,7);
        element.innerHTML = com.substr(8);
      } else {
        document.getElementById("hideBtn").hidden = com.substr(6,7);
        document.getElementById("icon").innerHTML = com.substr(8,9);
        document.getElementById("pin").innerHTML = com.substr(10,14);
        document.getElementById("title").innerHTML = com.substr(14);
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
    power = 0;
    powerShow();
  }*/
