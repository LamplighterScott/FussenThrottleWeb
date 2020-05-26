// Build HTML from ESP
/*
typedef struct {
  int id;  // system id number w/o system and type chars
  const char *title;  // any string for name of output
  const char *icon;  // icon
} fData;
// Format {id, title, icon}
fData ftt[]= {
  {0, "Light", "💡"},
  {1, "Beam", "🌞"},
  {2, "Cab 1", "1️⃣"},
  {3, "Cab 2", "2️⃣"},
  {4, "Shunt", "↔️"}
};
const int totalFuntions = 5;  // Must include the zero row at the end
*/

/*
typedef struct {
  int id;  // system id number w/o system and type chars
  const char *title;  // any string for name of output
  const char *icon;  // icon
} jData;
// Format {id, title, icon}
jData jtt[]= {
  {0, "Vol Up", "🔊"},
  {1, "Vol Dn", "🔉"},
  {2, "Horn", "🎺"},
  {3, "Approach", "💡"},
  {4, "Steam", "🚂"},
  {5, "Whistle", "🌬️"},
  {6, "Horn2", "📯"},
  {7, "Clear", "🚨"},
  {8, "Distant", "🛤️"},
  {8, "Crossing", "🛎️"},
  {9, "Air 1", "🚢"},
  {10, "Air 2", "⛴️"},
  {11, "Church", "⛪"},
  {12, "Clock", "🕰️"},
  {13, "Dixie", "💃"},
  {14, "Fire", "🚒"},
  {15, "Foghorn", "🌁"},
  {16, "Bell", "🔔"},
  {17, "Freight", "🚞"},
  {18, "HVAC", "🎐"},
  {19, "Street", "🚗"},
  {20, "Dog", "🐕"},
  {22, "Steam", "🚂"},
  {23, "Party", "🎉"},
  {24, "Idle", "⚙️"}
};
const int totalSounds = 24;  // Must include the zero row at the end
*/

//===============================================================
//   WEBPAGE BUILDER
//===============================================================

//void buildTO () {
//  for (tData &t: ttt) {

//    int state = t.zStatus;
//    String stateTxt = (state>0) ? "1" : "0";
//    String icon = (state>0) ? t.icon1 : t.icon0;
//    String itText = String(t.id);
//    char lineChar = "B" + stateTxt + "<li id=\"" + itText + "\" onClick=\"setZ('" + itText + "')\"> + t.icon + " " + t.title + "</li>";
//    webSocket.sendTXT(num, lineChar);
//   }

//}


<script src="FussenSetUpDeferJS.js" defer></script>
<script language="javascript" type="text/javascript" src="FussenSetUpJS.js"></script>


  <div class="drag" onclick="editItem(this)" id="C0">Cab 0</div>
  <div class="drag" onclick="editItem(this)" id="C1">Cab 1</div>
  <div class="drag" onclick="editItem(this)" id="C2">Cab 2</div>
  <div class="drag" onclick="editItem(this)" id="C3">Cab 3</div>
  <div class="drag" onclick="editItem(this)" id="C4">Cab 4</div>
  <div class="drag" onclick="editItem(this)" id="C5">Cab 5</div>
  <div class="drag" onclick="editItem(this)" id="C6">Cab 6</div>
  <div class="drag" onclick="editItem(this)" id="C7">Cab 7</div>
  <div class="drag" onclick="editItem(this)" id="C8">Cab 8</div>
  <div class="drag" onclick="editItem(this)" id="C9">Cab 9</div>

  <button type="button" class="btnL" data-dismiss="modal">↩️</button>
  <button type="button" class="btnR" data-dismiss="modal">💾</button>

<!-- Modal -->
<div id="myModal" class="modal fade" role="dialog">
  <div class="modal-dialog">

    <!-- Modal content-->
    <div class="modal-content">
      <div class="modal-header">
        <button type="button" class="close" data-dismiss="modal"style="font-size:30px;border-radius:10px;">↩️</button>
      </div>
      <div class="modal-body">
        <br><br>

        <form action="/cabEdit" type="text">
          <div style="color:blue;text-align:center;font-size:16px;">
          <label for="icon">Cab Icon</label><br>
          <input list="icons" id="icon" name="cabIcon" size="3" style="font-size:30px;">
          <datalist id="icons">
            <option value="🚂">
            <option value="🚃">
            <option value="🚋">
            <option value="🚊">
            <option value="🚄">
            <option value="🚅">
            <option value="🚞">
            <option value="🚅">
            <option value="🚇">
            <option value="🚅">
            <option value="🚝">
            <option value="🚈">
            <option value="🛺">
          </datalist><br><br>

          <label for="title">Cab Title id="title"</label><br>
          <label for="title">(15 characters max)</label><br>
          <input type="text" maxlength="15" id="title" name="title"  size="15" style="font-size:20px;"><br><br>

          <label for="number">DCC Encoded Cab Number id="number"</label><br>
          <label for="number">(1-999)</label><br>
          <input type"number max="999" id="number" name="number"  size="4" style="font-size:20px;"></input><br><br>
        </div>

        </form>
      </div>
      <div class="modal-footer">
        <button type="button" class="btn btn-default" data-dismiss="modal" style="font-size:30px;border-radius:10px;>💾</button>
      </div>
    </div>

  </div>
</div>
