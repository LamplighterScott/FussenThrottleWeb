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
