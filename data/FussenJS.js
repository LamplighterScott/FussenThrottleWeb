var curLoco = "None";
var power = 0;
var speed = 0;
var direction = 1; // forward
var clicked = 0;
var sendText = "";

function selectLoco() {
  var fID = this.id;
  var tID = fID.substr(1);
  var loco = Number(tID);
  if (curLoco == loco) {
    document.getElementById("locoHeader").innerHTML = "Select Loco";
    curLoco = "None";
  } else {
    document.getElementById("locoHeader").innerHTML = document.getElementById(loco).innerHTML;
    curLoco = loco;
    sendText = "<s " + curLoco + ">";
    connection.send(sendText);
  }
}

function showLocos() {
  document.getElementById("myDropdown").classList.toggle("show");
}

// Close the dropdown if the user clicks outside of it
window.onclick = function(event) {
  if (!event.target.matches('.dropbtn')) {
    var dropdowns = document.getElementsByClassName("dropdown-content");
    var i;
    for (i = 0; i < dropdowns.length; i++) {
      var openDropdown = dropdowns[i];
      if (openDropdown.classList.contains('show')) {
        openDropdown.classList.remove('show');
      }
    }
  }
}

function setZ(idTxt) {
  if (power>0) {
    writeToScreen(idTxt);
    sendText = "<Z " + idTxt + ">";
    connection.send(sendText);
  } else {
    sendText = "Power is off";
    writeToScreen(sendText);
  }
}

function setF() {
  if (curLoco != "None") {
    var fID = this.id;
    writeToScreen(fID);  //  Eliminate this !!!!!!!!!!!!
    var tID = fID.substr(1);
    sendText = "<F " + curLoco + " " + tID + ">";
    connection.send(sendText);
  } else {
    sendText = "No loco selected!";
    writeToScreen(sendText);
  }
}


function sendPower() {
  power = Math.abs(power-1);
  sendText = (power>0) ? "<1>" : "<0>";
  connection.send(sendText);
}

function showPower() {
  sendText = (power>0) ? "⚡" : "⭕";
  document.getElementById("powerButton").innerHTML = sendText;
}


function setDir (Sdir) {
	clicked++;
	if (clicked<2) {
		setTimeout(function() {
			if (clicked===1) {
				direction = Sdir;
			} else {
				if (Sdir > 0) {
					speed += 13;
				} else {
					speed -= 13;
				}
			}
			clicked=0;
			sendSpeed(1);
		}, 300);
	}
}


function setSpeed(getSpeed) {
  if (getSpeed > 0) {
    speed = document.getElementById("speedSlider").value;
  }
  if (speed > 0) {
    direction = 1;
  } else {
    direction = 0;
  }
  sendSpeed(1);
}

function stopSpeed() {
	clicked++;
	if (clicked<2) {
		setTimeout(function() {
			if (clicked===1) {
				clicked=0;
				sendSpeed(0);
			} else {
				clicked=0;
				sendSpeed(-1);
			}
		}, 300);
	}
 }

function sendSpeed(param) {
  if (param>0) {
    speed = Math.abs(speed);
  } else if (param<0) {
    speed = -1;
  } else {
    speed = 0;
  }
  if (curLoco != "None") {
    var directionTxt = direction.toString();
    var speedTxt = speed.toString();
    sendText = "<t " + curLoco + " " + speedTxt + " " + directionTxt + ">";
    connection.send(sendText);
  }
}

function writeToScreen(message) {
  var output = document.getElementById("output");
  message = "--" + message;
  output.innerHTML+= message;
}

function setCharAt(str,index,chr) {
    if(index > str.length-1) return str;
    return str.substr(0,index) + chr + str.substr(index+1);
}


function onMessage(event) {
  var messageTxt = event.data;
  var last = messageTxt.lastIndexOf(">");
  var com = messageTxt.substr(1,last-1);
  var com1 = com.substr(0,1);
  switch (com1) {
    case "0": {
      power = 0;
      showPower();
      break;
    }
    case "1": {
      power = 1;
      showPower();
      break;
    }
    case "t": {
      var Tarray = com.split(" ");
      var RcabTxt = Tarray[1];
      if (RcabTxt == curLoco) {
        var RspeedTxt = Tarray[2];
        var RdirectionTxt = Tarray[3];
        speed = Number(RspeedTxt);
        direction = Number(RdirectionTxt);
        if (speed<0) {
          speed = 0;
        } else {
          if (direction<1) {
            speed = -speed;
          }
        }
        var slider = document.getElementById("speedSlider");
        slider.value = speed;
      }
      break;
    }
    case "Y": {
      var zArray = com.split(" ");
      var rID = zArray[1];
      var rStateTxt = zArray[2];
      var state = Number(RstateTxt);
      var rIcon = zArray[3];
      var innerTxt = document.getElementById(rID).innerHTML;
      var innerArray = Array.from(innerTxt);
      innerArray[0] = Ricon;
      document.getElementById(rID).innerHTML = innerArray.join("");
      var textColor = "color:green";
      if (state>0) {
        textColor = "color:red";
      }
      document.getElementById(Rid).style = textColor;
      break;
    }
    case "T": {
      var avant = com.substr(0,7);
      var zArray = avant.split(" ");
      var rID = zArray[0];
      var rStateTxt = zArray[1];
      var state = Number(RstateTxt);
      var hide = zArray[2]
      var iconTitle = com.substr(8);
      var element = document.getElementById(rID);
      element.innerHTML = iconTitle;
      var textColor = "color:green";
      if (state>0) {
        textColor = "color:red";
      }
      element.style = textColor;
      if (hide>0) {
        element.setAttribute("hidden", "false");
      } else {
        element.removeAttribute("hidden");
      }
      break;
    }
    case "C": {
      var avant = com.substr(0,5);
      var zArray = avant.split(" ");
      var rID = zArray[0];
      var hide = Number(zArray[1]);
      var iconTitle = com.substr(6);
      var element = document.getElementById(rID);
      element.innerHTML = iconTitle;
      if (hide>0) {
        element.setAttribute("hidden", "false");
      } else {
        element.removeAttribute("hidden");
      }
      break;
    }
  }
}

/*
var connection = new WebSocket("ws://" + location.hostname + ":81/", ['arduino']);

connection.onopen = function () {
  connection.send('Connect ' + new Date());
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
