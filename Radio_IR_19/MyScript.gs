function  onOpen() {
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  var menuEntries = [
                       {name: "Сортировка по дате (уб)",  functionName: "SortirovkaUp"},
                       {name: "Сортировка по дате (воз)",  functionName: "SortirovkaDown"},
                  //     {name: "Monochrome (White/High)", functionName: "bToW"},
                  //     {name: "Red (Low) to Yellow (High)  Font",  functionName: "rToYFont"},
                  //    {name: "Red (Low) to Yellow (High)", functionName: "rToY"}
                     ];
  ss.addMenu("Мои скрипты", menuEntries);
 
}

/**
 * Automatically sorts the 2cond column (not the header row) Ascending.
 */
function SortirovkaUp() {
var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sheet = ss.getSheets()[0];
    // Sorts the sheet by the first column, ascending
   sheet.sort(2,false);

    var cell = sheet.getRange("E2:1000"); // столбец E2
    cell.setWrap(true);           // включить перенос по словам
    var cell = sheet.getRange("F2:1000"); // столбец F2
    cell.setWrap(true);           // включить перенос по словам

 }
 
/**
 * Automatically sorts the 2cond column (not the header row) Ascending.
 */
function SortirovkaDown() {
var ss = SpreadsheetApp.getActiveSpreadsheet();
 var sheet = ss.getSheets()[0];
  // Sorts the sheet by the first column, ascending
 sheet.sort(2,true);  

  var cell = sheet.getRange("E2:1000"); // столбец E2
    cell.setWrap(true);           // включить перенос по словам
  var cell = sheet.getRange("F2:1000"); // столбец F2
    cell.setWrap(true);           // включить перенос по словам

 }
 
