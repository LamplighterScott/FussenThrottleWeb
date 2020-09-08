var curLoco = "C00";
var power = 0;
var speed = 0;
var direction = 1; // forward
var clicked = 0;
var sendText = "";

function initElements() {
  setTimeout(function() {
    const tElement = document.getElementById("turns");
    const tRows = tElement.querySelectorAll("*");
    const cElement = document.getElementById("myDropdown");
    const cRows = cElement.querySelectorAll("*");
    const fElement = document.getElementById("fLoco");
    const fRows = fElement.querySelectorAll("*");
    const pElement = document.getElementById("pSounds");
    const pRows = pElement.querySelectorAll("*");

    tRows.forEach(function(item){
      item.addEventListener("click", function() {
        setZ(item.id);
      });
      getInner(item);
    });

    cRows.forEach(function(item){
      item.addEventListener("click", function() {
        selectLoco(item.id);
      });
      getInner(item);
    });

    fRows.forEach(function(item){
      item.addEventListener("click", function() {
        setZ(item.id);
      });
    });

    pRows.forEach(function(item){
      item.addEventListener("click", function() {
        setZ(item.id);
      });
    });

    var w = window.innerHeight-210;
    sendText = w.toString() + "px";
    for (let e of document.querySelectorAll ("ul")) {
      e.style.height=sendText;
    }

    connection.send("<C>");
    document.getElementById("locoHeader").innerHTML = document.getElementById(curLoco).innerHTML;
  }, 200);
}

function getInner(item) {
    const tID = item.id;
    sendText = "<Z " + tID + ">";
    connection.send(sendText);
}

function selectLoco(fID) {
  document.getElementById("locoHeader").innerHTML = document.getElementById(fID).innerHTML;
  curLoco = fID;
  sendText = "<" + curLoco + ">";
  connection.send(sendText);
}

function showLocos() {
  document.getElementById("myDropdown").classList.toggle("show");
}

// Close the dropdown if the user clicks outside of it
window.onclick = function(event) {
  if (!event.target.matches('.dropbtn')) {
    const dropdowns = document.getElementsByClassName("dropdown-content");
    let i;
    for (i = 0; i < dropdowns.length; i++) {
      var openDropdown = dropdowns[i];
      if (openDropdown.classList.contains('show')) {
        openDropdown.classList.remove('show');
      }
    }
  }
}

function setZ(tID) {
  
    writeToScreen(tID);
    const com = tID[0];
    if (com == "F" ) {
        sendText = "<" + tID + ">";
        connection.send(sendText);
    } else if (com == "T") {
      var id = tID.substr(1);
      sendText = "<Z " + "A" + id + ">";
      connection.send(sendText);
    } else if (com == "J") {
      sendText = "<Z " + tID + ">";
      connection.send(sendText);
    }
  
  writeToScreen(sendText);
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
  const directionTxt = direction.toString();
  const speedTxt = speed.toString();
  sendText = "<t " + curLoco + " " + speedTxt + " " + directionTxt + ">";
  connection.send(sendText);
  
}

function writeToScreen(message) {
  const output = document.getElementById("output");
  message = "--" + message;
  output.innerHTML+= message;
}

function setCharAt(str,index,chr) {
    if(index > str.length-1) return str;
    return str.substr(0,index) + chr + str.substr(index+1);
}

function onMessage(event) {
  const messageTxt = event.data;
  const last = messageTxt.lastIndexOf(">");
  const com = messageTxt.substr(1,last-1);
  const com1 = com.substr(0,1);
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
      const tArray = com.split(" ");
      var upLoco = `C0${tArray[1]}`;
      writeToScreen(upLoco);
      if (upLoco != curLoco) {
        curLoco=upLoco;
        document.getElementById("locoHeader").innerHTML = document.getElementById(curLoco).innerHTML;
      }
      const rSpeedTxt = tArray[2];
      const rDirectionTxt = tArray[3];
      speed = Number(rSpeedTxt);
      direction = Number(rDirectionTxt);
      if (speed<0) {
        speed = 0;
      } else {
        if (direction<1) {
          speed = -speed;
        }
      }
      const slider = document.getElementById("speedSlider");
      slider.value = speed;
      break;
    }
    case "Y": {
      const zArray = com.split(" ");
      const rID = zArray[1];
      const element = document.getElementById(rID);
      const rStateTxt = zArray[2];
      const state = Number(rStateTxt);
      const rIcon = zArray[3];
      const innerTxt = element.innerHTML;
      let innerArray = Array.from(innerTxt);
      innerArray[0] = rIcon;
      element.innerHTML = innerArray.join("");
      let textColor = "color:red";
      if (state>0) {
        textColor = "color:green";
      }
      element.style = textColor;
      break;
    }
    case "T": {
      const avant = com.substr(2,7);
      const zArray = avant.split(" ");
      const rID = zArray[0];
      const rStateTxt = zArray[1];
      const state = Number(rStateTxt);
      const show = Number(zArray[2]);
      const iconTitle = com.substr(10);
      const element = document.getElementById(rID);
      element.innerHTML = iconTitle;
      let textColor = "color:red";
      if (state>0) {
        textColor = "color:green";
      }
      element.style = textColor;
      if (show>0) {
        element.removeAttribute("hidden");
      } else {
        element.setAttribute("hidden", "false");
      }
      break;
    }
    case "C": {
      const avant = com.substr(2,5);
      const zArray = avant.split(" ");
      const rID = zArray[0];
      const show = Number(zArray[1]);
      const iconTitle = com.substr(8);
      const element = document.getElementById(rID);
      element.innerHTML = iconTitle;
      if (show>0) {
        // element.removeAttribute("hidden");
        element.style.display = "block";
      } else {
        //element.setAttribute("hidden", "false");
        element.style.display = "none";
      }
      break;
    }
  }
}

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
  showPower();
}
