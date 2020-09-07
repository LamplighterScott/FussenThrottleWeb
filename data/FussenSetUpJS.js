var sendTxt = "";
var dropped;
var max = 15;
var DID; // Dragged ID
var DIH; // Dragged innerHTML
var LDO; // Last Dragged Over ID
var LDH; // Last Dragged Over innerHTML

function showTurnouts() {
    max = 15;
    document.getElementById("cHide").setAttribute("hidden", "false");
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

var tRows;
var cRows;
var modal;

function initElements() {
    setTimeout(sendInit, 200);
    function sendInit() {
        modal = document.getElementById("myModal");
        let tElement = document.getElementById("turnouts");
        tRows = tElement.querySelectorAll(".drag");
        let cElement = document.getElementById("cabs");
        cRows = cElement.querySelectorAll(".drag");
        tRows.forEach(addEventListeners);
        cRows.forEach(addEventListeners);
    }

    function addEventListeners(item) {
        item.setAttribute("draggable", "true");
        item.addEventListener("click", function() {
            editItem(item.id);
        });
        item.addEventListener("dragstart", dragStart);
        item.addEventListener("dragend", dragEnd);
        item.addEventListener("dragover", dragOver);
        item.addEventListener("dragenter", dragEnter);
        item.addEventListener("dragleave", dragLeave);
        item.addEventListener("drop", drop);

        sendTxt = "<I " + item.id + ">";
        connection.send(sendTxt);
            
    }

    let itElement = document.getElementById("iconDropS");
    let ttIcons = itElement.querySelectorAll("*");
    ttIcons.forEach(addIconListeners);
    let icElement = document.getElementById("iconDropB");
    let tbIcons = icElement.querySelectorAll("*");
    tbIcons.forEach(addIconListeners);

    function addIconListeners(item) {
        item.addEventListener("click", function() {
            selectIcon(item.innerHTML);
        });
    }

}

function editItem(tID) {
    modal.style.display = "block";
    DID = tID;
    let eTitle = document.getElementById("tTitle");
    let eType = document.getElementById("typeGrp");
    let eNumber = document.getElementById("pinTitle");

    if (max > 10) {
        eType.removeAttribute("hidden");
        eNumber.innerHTML = "MEGA Pin Number";
    } else {
        eTitle.innerHTML = "Loco Name"
        eType.setAttribute("hidden", "false");
        eNumber.innerHTML = "DCC Encoded Cab Number"
    }

    sendTxt = "<M " + DID + ">";
    connection.send(sendTxt);
}

function dragStart(event) {
    var t = event.target;
    event.dataTransfer.setData("text", t.innerHTML);
    DID = LDO = t.id;
    DIH = t.innerHTML;
    t.style.opacity = .5;
    dropped = false;
    event.stopPropagation();
}

function dragEnd(event) {
    var t = event.target;
    if (t.className = "drag") {
        t.style.opacity = "";
        // Reset innerHTML if dragged out of bounds
        if (dropped == false) {
            var DIDN = Number(DID.substr(1));
            var LDON = Number(LDO.substr(1));
            if (max > 10) {
                resetRows(tRows);
            } else {
                resetRows(cRows);
            }

            function resetRows(rows) {
                if (LDON > 0) { // shift up
                    for (x = max - 1; x > DIDN - 1; x--) {
                        rows[x].innerHTML = rows[x - 1].innerHTML;
                        rows[x].color = rows[x - 1].color;
                    }
                } else { // shift down
                    for (x = 0; x < DIDN; x++) {
                        rows[x].innerHTML = rows[x + 1].innerHTML;
                        rows[x].color = rows[x + 1].color;
                    }
                }
                rows[DIDN].innerHTML = DIH;
            }
        } else {
            sendTxt = "<I " + DID + " " + LDO + ">";
            connection.send(sendTxt);
        }
    }
    event.stopPropagation();
}

/* events fired on the drop targets */
function dragEnter(event) {
    // highlight potential drop target when the draggable element enters it
    var t = event.target;
    if (t.className = "drag") {
        t.style.background = "purple";
        LDH = t.innerHTML;
        t.innerHTML = "- - -";
    }
    event.stopPropagation();
}

function dragOver(event) {
    // prevent default to allow drop
    event.preventDefault();
    LDO = event.target.id;
    event.stopPropagation();
}

function dragLeave(event) {
    // reset background of potential drop target when the draggable element leaves it
    var t = event.target;
    if (t.className == "drag") {
        t.style.background = "";
        t.innerHTML = LDH;
    }
    event.stopPropagation();
}

function drop(event) {
    // prevent default action (open as link for some elements)
    event.preventDefault();
    var t = event.target;
    if (t.className == "drag") {
        t.style.background = "";
        t.innerHTML = event.dataTransfer.getData("text");
        dropped = true;
    }
    event.stopPropagation();
}

// Modal section


function showLine(el) {
    if (el.innerHTML == "👁️") {
        el.innerHTML = "💤";
    } else {
        el.innerHTML = "👁️";
    }
}

var rx = /([\uD800-\uDBFF][\uDC00-\uDFFF](?:[\u200D\uFE0F][\uD800-\uDBFF][\uDC00-\uDFFF]){2,}|\uD83D\uDC69(?:\u200D(?:(?:\uD83D\uDC69\u200D)?\uD83D\uDC67|(?:\uD83D\uDC69\u200D)?\uD83D\uDC66)|\uD83C[\uDFFB-\uDFFF])|\uD83D\uDC69\u200D(?:\uD83D\uDC69\u200D)?\uD83D\uDC66\u200D\uD83D\uDC66|\uD83D\uDC69\u200D(?:\uD83D\uDC69\u200D)?\uD83D\uDC67\u200D(?:\uD83D[\uDC66\uDC67])|\uD83C\uDFF3\uFE0F\u200D\uD83C\uDF08|(?:\uD83C[\uDFC3\uDFC4\uDFCA]|\uD83D[\uDC6E\uDC71\uDC73\uDC77\uDC81\uDC82\uDC86\uDC87\uDE45-\uDE47\uDE4B\uDE4D\uDE4E\uDEA3\uDEB4-\uDEB6]|\uD83E[\uDD26\uDD37-\uDD39\uDD3D\uDD3E\uDDD6-\uDDDD])(?:\uD83C[\uDFFB-\uDFFF])\u200D[\u2640\u2642]\uFE0F|\uD83D\uDC69(?:\uD83C[\uDFFB-\uDFFF])\u200D(?:\uD83C[\uDF3E\uDF73\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92])|(?:\uD83C[\uDFC3\uDFC4\uDFCA]|\uD83D[\uDC6E\uDC6F\uDC71\uDC73\uDC77\uDC81\uDC82\uDC86\uDC87\uDE45-\uDE47\uDE4B\uDE4D\uDE4E\uDEA3\uDEB4-\uDEB6]|\uD83E[\uDD26\uDD37-\uDD39\uDD3C-\uDD3E\uDDD6-\uDDDF])\u200D[\u2640\u2642]\uFE0F|\uD83C\uDDFD\uD83C\uDDF0|\uD83C\uDDF6\uD83C\uDDE6|\uD83C\uDDF4\uD83C\uDDF2|\uD83C\uDDE9(?:\uD83C[\uDDEA\uDDEC\uDDEF\uDDF0\uDDF2\uDDF4\uDDFF])|\uD83C\uDDF7(?:\uD83C[\uDDEA\uDDF4\uDDF8\uDDFA\uDDFC])|\uD83C\uDDE8(?:\uD83C[\uDDE6\uDDE8\uDDE9\uDDEB-\uDDEE\uDDF0-\uDDF5\uDDF7\uDDFA-\uDDFF])|(?:\u26F9|\uD83C[\uDFCB\uDFCC]|\uD83D\uDD75)(?:\uFE0F\u200D[\u2640\u2642]|(?:\uD83C[\uDFFB-\uDFFF])\u200D[\u2640\u2642])\uFE0F|(?:\uD83D\uDC41\uFE0F\u200D\uD83D\uDDE8|\uD83D\uDC69(?:\uD83C[\uDFFB-\uDFFF])\u200D[\u2695\u2696\u2708]|\uD83D\uDC69\u200D[\u2695\u2696\u2708]|\uD83D\uDC68(?:(?:\uD83C[\uDFFB-\uDFFF])\u200D[\u2695\u2696\u2708]|\u200D[\u2695\u2696\u2708]))\uFE0F|\uD83C\uDDF2(?:\uD83C[\uDDE6\uDDE8-\uDDED\uDDF0-\uDDFF])|\uD83D\uDC69\u200D(?:\uD83C[\uDF3E\uDF73\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]|\u2764\uFE0F\u200D(?:\uD83D\uDC8B\u200D(?:\uD83D[\uDC68\uDC69])|\uD83D[\uDC68\uDC69]))|\uD83C\uDDF1(?:\uD83C[\uDDE6-\uDDE8\uDDEE\uDDF0\uDDF7-\uDDFB\uDDFE])|\uD83C\uDDEF(?:\uD83C[\uDDEA\uDDF2\uDDF4\uDDF5])|\uD83C\uDDED(?:\uD83C[\uDDF0\uDDF2\uDDF3\uDDF7\uDDF9\uDDFA])|\uD83C\uDDEB(?:\uD83C[\uDDEE-\uDDF0\uDDF2\uDDF4\uDDF7])|[#\*0-9]\uFE0F\u20E3|\uD83C\uDDE7(?:\uD83C[\uDDE6\uDDE7\uDDE9-\uDDEF\uDDF1-\uDDF4\uDDF6-\uDDF9\uDDFB\uDDFC\uDDFE\uDDFF])|\uD83C\uDDE6(?:\uD83C[\uDDE8-\uDDEC\uDDEE\uDDF1\uDDF2\uDDF4\uDDF6-\uDDFA\uDDFC\uDDFD\uDDFF])|\uD83C\uDDFF(?:\uD83C[\uDDE6\uDDF2\uDDFC])|\uD83C\uDDF5(?:\uD83C[\uDDE6\uDDEA-\uDDED\uDDF0-\uDDF3\uDDF7-\uDDF9\uDDFC\uDDFE])|\uD83C\uDDFB(?:\uD83C[\uDDE6\uDDE8\uDDEA\uDDEC\uDDEE\uDDF3\uDDFA])|\uD83C\uDDF3(?:\uD83C[\uDDE6\uDDE8\uDDEA-\uDDEC\uDDEE\uDDF1\uDDF4\uDDF5\uDDF7\uDDFA\uDDFF])|\uD83C\uDFF4\uDB40\uDC67\uDB40\uDC62(?:\uDB40\uDC77\uDB40\uDC6C\uDB40\uDC73|\uDB40\uDC73\uDB40\uDC63\uDB40\uDC74|\uDB40\uDC65\uDB40\uDC6E\uDB40\uDC67)\uDB40\uDC7F|\uD83D\uDC68(?:\u200D(?:\u2764\uFE0F\u200D(?:\uD83D\uDC8B\u200D)?\uD83D\uDC68|(?:(?:\uD83D[\uDC68\uDC69])\u200D)?\uD83D\uDC66\u200D\uD83D\uDC66|(?:(?:\uD83D[\uDC68\uDC69])\u200D)?\uD83D\uDC67\u200D(?:\uD83D[\uDC66\uDC67])|\uD83C[\uDF3E\uDF73\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92])|(?:\uD83C[\uDFFB-\uDFFF])\u200D(?:\uD83C[\uDF3E\uDF73\uDF93\uDFA4\uDFA8\uDFEB\uDFED]|\uD83D[\uDCBB\uDCBC\uDD27\uDD2C\uDE80\uDE92]))|\uD83C\uDDF8(?:\uD83C[\uDDE6-\uDDEA\uDDEC-\uDDF4\uDDF7-\uDDF9\uDDFB\uDDFD-\uDDFF])|\uD83C\uDDF0(?:\uD83C[\uDDEA\uDDEC-\uDDEE\uDDF2\uDDF3\uDDF5\uDDF7\uDDFC\uDDFE\uDDFF])|\uD83C\uDDFE(?:\uD83C[\uDDEA\uDDF9])|\uD83C\uDDEE(?:\uD83C[\uDDE8-\uDDEA\uDDF1-\uDDF4\uDDF6-\uDDF9])|\uD83C\uDDF9(?:\uD83C[\uDDE6\uDDE8\uDDE9\uDDEB-\uDDED\uDDEF-\uDDF4\uDDF7\uDDF9\uDDFB\uDDFC\uDDFF])|\uD83C\uDDEC(?:\uD83C[\uDDE6\uDDE7\uDDE9-\uDDEE\uDDF1-\uDDF3\uDDF5-\uDDFA\uDDFC\uDDFE])|\uD83C\uDDFA(?:\uD83C[\uDDE6\uDDEC\uDDF2\uDDF3\uDDF8\uDDFE\uDDFF])|\uD83C\uDDEA(?:\uD83C[\uDDE6\uDDE8\uDDEA\uDDEC\uDDED\uDDF7-\uDDFA])|\uD83C\uDDFC(?:\uD83C[\uDDEB\uDDF8])|(?:\u26F9|\uD83C[\uDFCB\uDFCC]|\uD83D\uDD75)(?:\uD83C[\uDFFB-\uDFFF])|(?:\uD83C[\uDFC3\uDFC4\uDFCA]|\uD83D[\uDC6E\uDC71\uDC73\uDC77\uDC81\uDC82\uDC86\uDC87\uDE45-\uDE47\uDE4B\uDE4D\uDE4E\uDEA3\uDEB4-\uDEB6]|\uD83E[\uDD26\uDD37-\uDD39\uDD3D\uDD3E\uDDD6-\uDDDD])(?:\uD83C[\uDFFB-\uDFFF])|(?:[\u261D\u270A-\u270D]|\uD83C[\uDF85\uDFC2\uDFC7]|\uD83D[\uDC42\uDC43\uDC46-\uDC50\uDC66\uDC67\uDC70\uDC72\uDC74-\uDC76\uDC78\uDC7C\uDC83\uDC85\uDCAA\uDD74\uDD7A\uDD90\uDD95\uDD96\uDE4C\uDE4F\uDEC0\uDECC]|\uD83E[\uDD18-\uDD1C\uDD1E\uDD1F\uDD30-\uDD36\uDDD1-\uDDD5])(?:\uD83C[\uDFFB-\uDFFF])|\uD83D\uDC68(?:\u200D(?:(?:(?:\uD83D[\uDC68\uDC69])\u200D)?\uD83D\uDC67|(?:(?:\uD83D[\uDC68\uDC69])\u200D)?\uD83D\uDC66)|\uD83C[\uDFFB-\uDFFF])|(?:[\u261D\u26F9\u270A-\u270D]|\uD83C[\uDF85\uDFC2-\uDFC4\uDFC7\uDFCA-\uDFCC]|\uD83D[\uDC42\uDC43\uDC46-\uDC50\uDC66-\uDC69\uDC6E\uDC70-\uDC78\uDC7C\uDC81-\uDC83\uDC85-\uDC87\uDCAA\uDD74\uDD75\uDD7A\uDD90\uDD95\uDD96\uDE45-\uDE47\uDE4B-\uDE4F\uDEA3\uDEB4-\uDEB6\uDEC0\uDECC]|\uD83E[\uDD18-\uDD1C\uDD1E\uDD1F\uDD26\uDD30-\uDD39\uDD3D\uDD3E\uDDD1-\uDDDD])(?:\uD83C[\uDFFB-\uDFFF])?|(?:[\u231A\u231B\u23E9-\u23EC\u23F0\u23F3\u25FD\u25FE\u2614\u2615\u2648-\u2653\u267F\u2693\u26A1\u26AA\u26AB\u26BD\u26BE\u26C4\u26C5\u26CE\u26D4\u26EA\u26F2\u26F3\u26F5\u26FA\u26FD\u2705\u270A\u270B\u2728\u274C\u274E\u2753-\u2755\u2757\u2795-\u2797\u27B0\u27BF\u2B1B\u2B1C\u2B50\u2B55]|\uD83C[\uDC04\uDCCF\uDD8E\uDD91-\uDD9A\uDDE6-\uDDFF\uDE01\uDE1A\uDE2F\uDE32-\uDE36\uDE38-\uDE3A\uDE50\uDE51\uDF00-\uDF20\uDF2D-\uDF35\uDF37-\uDF7C\uDF7E-\uDF93\uDFA0-\uDFCA\uDFCF-\uDFD3\uDFE0-\uDFF0\uDFF4\uDFF8-\uDFFF]|\uD83D[\uDC00-\uDC3E\uDC40\uDC42-\uDCFC\uDCFF-\uDD3D\uDD4B-\uDD4E\uDD50-\uDD67\uDD7A\uDD95\uDD96\uDDA4\uDDFB-\uDE4F\uDE80-\uDEC5\uDECC\uDED0-\uDED2\uDEEB\uDEEC\uDEF4-\uDEF8]|\uD83E[\uDD10-\uDD3A\uDD3C-\uDD3E\uDD40-\uDD45\uDD47-\uDD4C\uDD50-\uDD6B\uDD80-\uDD97\uDDC0\uDDD0-\uDDE6])|(?:[#\*0-9\xA9\xAE\u203C\u2049\u2122\u2139\u2194-\u2199\u21A9\u21AA\u231A\u231B\u2328\u23CF\u23E9-\u23F3\u23F8-\u23FA\u24C2\u25AA\u25AB\u25B6\u25C0\u25FB-\u25FE\u2600-\u2604\u260E\u2611\u2614\u2615\u2618\u261D\u2620\u2622\u2623\u2626\u262A\u262E\u262F\u2638-\u263A\u2640\u2642\u2648-\u2653\u2660\u2663\u2665\u2666\u2668\u267B\u267F\u2692-\u2697\u2699\u269B\u269C\u26A0\u26A1\u26AA\u26AB\u26B0\u26B1\u26BD\u26BE\u26C4\u26C5\u26C8\u26CE\u26CF\u26D1\u26D3\u26D4\u26E9\u26EA\u26F0-\u26F5\u26F7-\u26FA\u26FD\u2702\u2705\u2708-\u270D\u270F\u2712\u2714\u2716\u271D\u2721\u2728\u2733\u2734\u2744\u2747\u274C\u274E\u2753-\u2755\u2757\u2763\u2764\u2795-\u2797\u27A1\u27B0\u27BF\u2934\u2935\u2B05-\u2B07\u2B1B\u2B1C\u2B50\u2B55\u3030\u303D\u3297\u3299]|\uD83C[\uDC04\uDCCF\uDD70\uDD71\uDD7E\uDD7F\uDD8E\uDD91-\uDD9A\uDDE6-\uDDFF\uDE01\uDE02\uDE1A\uDE2F\uDE32-\uDE3A\uDE50\uDE51\uDF00-\uDF21\uDF24-\uDF93\uDF96\uDF97\uDF99-\uDF9B\uDF9E-\uDFF0\uDFF3-\uDFF5\uDFF7-\uDFFF]|\uD83D[\uDC00-\uDCFD\uDCFF-\uDD3D\uDD49-\uDD4E\uDD50-\uDD67\uDD6F\uDD70\uDD73-\uDD7A\uDD87\uDD8A-\uDD8D\uDD90\uDD95\uDD96\uDDA4\uDDA5\uDDA8\uDDB1\uDDB2\uDDBC\uDDC2-\uDDC4\uDDD1-\uDDD3\uDDDC-\uDDDE\uDDE1\uDDE3\uDDE8\uDDEF\uDDF3\uDDFA-\uDE4F\uDE80-\uDEC5\uDECB-\uDED2\uDEE0-\uDEE5\uDEE9\uDEEB\uDEEC\uDEF0\uDEF3-\uDEF8]|\uD83E[\uDD10-\uDD3A\uDD3C-\uDD3E\uDD40-\uDD45\uDD47-\uDD4C\uDD50-\uDD6B\uDD80-\uDD97\uDDC0\uDDD0-\uDDE6])\uFE0F)/;

function saveEdit() {
    const icon = document.getElementById("iconBtn").innerHTML;
    let iconArr = icon.split(rx).filter(Boolean);
    let icon0 = iconArr[0];
    let icon1 = icon0;
    if (iconArr.length > 1) {
        icon1 = iconArr[1];
    }

    const rbs = document.querySelectorAll('input[name="typeChoice"]');
    let type = "T";
    for (const rb of rbs) {
        if (rb.checked) {
            type = rb.value;
            break;
        }
    }
    const pin = document.getElementById("pin").value;
    const checked = document.getElementById("btnL").innerHTML;
    const show = (checked == "👁️") ? 1 : 0;
    sendTxt = "<K " + DID + " " + show + " " + type + " " + pin + " " + icon0 + " " +  icon1 + ">";
    modal.style.display = "none";
    connection.send(sendTxt);
    
    const title = document.getElementById("title").value;
    sendTxt = "<L " + DID + " " + title + ">";
    connection.send(sendTxt);
    sendTxt = "<I " + DID + ">";
    connection.send(sendTxt);
}

window.onclick = function(event) {
    if (event.target.id == "modal") {
        modal.style.display = "none";
    /*} else if (!event.target.matches('.dropbtn')) {*/
    } else if (event.target.matches('.dropdown')||!event.target.matches('.dropbtn')) {
        let dropdowns = document.getElementsByClassName("dropdown-content");
        let i;
        for (i = 0; i < dropdowns.length; i++) {
            let openDropdown = dropdowns[i];
            if (openDropdown.classList.contains('show')) {
                openDropdown.classList.remove('show');
            }
        }
    }
}

function dismisModal() {
    modal.style.display = "none";
}


function selectIcon(icon) {
    document.getElementById("iconBtn").innerHTML = icon;
    let iconArr = icon.split(rx).filter(Boolean);
    let elT = document.getElementById("T")
    let elS = document.getElementById("S")
;    if (iconArr.length > 1) {
        elT.disabled = false;
        elS.disabled = false;
        elT.checked = true;
        document.getElementById("D").disabled = true;
        document.getElementById("L").disabled = true;
    } else {
        elT.disabled = true;
        elS.disabled = true;
        document.getElementById("D").checked = true;
        document.getElementById("D").disabled = false;
        document.getElementById("L").disabled = false;
    }
}

function showIcons() {
    let tHide = document.getElementById("tHide").hidden;
    let eB = document.getElementById("iconDropB");
    let eS = document.getElementById("iconDropS")
    if (tHide > 0) {
        eB.classList.toggle("show");
    } else {
        eS.classList.toggle("show");
    }
}


// Communication

var connection = new WebSocket("ws://" + location.hostname + ":81/", ['arduino']);
let textArr;

function onMessage(event) {
    var messageTxt = event.data;
    var last = messageTxt.lastIndexOf(">");
    var com = messageTxt.substr(1, last - 1);
    var com1 = com.substr(0, 1);
    
    if (modal.style.display === "block") {
        if (com1 == "K") {
            textArr = com.split(" ");
            let showL = Number(textArr[1]);
            let el = document.getElementById("btnL");
            if (showL > 0) {
                el.innerHTML = "👁️";
            } else {
                el.innerHTML = "💤";
            }
            document.getElementById("pin").value = textArr[2];
            document.getElementById("iconBtn").innerHTML = textArr[3];
            if (textArr.length > 4) {  // Outputs
                let type = textArr[4];
                let typeEl = document.getElementById(type);
                if (type == "T" || type == "S") {
                    typeEl.disabled = false;
                    document.getElementById("D").disabled = true;
                    document.getElementById("L").disabled = true;
                } else if (type == "S") {
                    document.getElementById("T").disabled = true;
                    document.getElementById("S").disabled = true;
                }
                typeEl.checked = true;
            }
            
        } else if (com1 == "L") {
            document.getElementById("title").value = com.substr(2);
        }

    } else {
        
        let tID = com.substr(2, 3);
        let element = document.getElementById(tID);
        let show = Number(com.substr(6, 1));
        let pin = com.substr(8, 3);
        if (show > 0) {
            pin = "👁️  " + pin + "  ";
        } else {
            pin = "💤  " + pin + "  ";
        }
        let innerText = pin + com.substr(12);
        element.innerHTML = innerText;
    }
}

connection.onopen = function() {
    connection.send('Connect ' + new Date());

}

connection.onerror = function(error) {
    console.log('WebSocket Error ', error);
}

connection.onmessage = function(event) {
    console.log('Server: ', event.data);
    onMessage(event);

}

connection.onclose = function() {
    console.log('WebSocket connection closed');
}