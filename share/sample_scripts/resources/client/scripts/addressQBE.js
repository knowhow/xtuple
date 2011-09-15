/* A sample parent window and script showing how to open scripted
   child windows. this demonstrates how to open child windows
   and pass data from the parent to the children when the children
   .ui form definitions have been created as QMainWindows, QDialogs,
   and QWidgets. children with QDialog .ui definitions allow the parent
   to use accept() and reject() to pass simple data from the child
   back to the parent.
*/

mywindow.findChild("_close").clicked.connect(mywindow, "close");

var _searchMe = mywindow.findChild("_searchMe");
var _found    = mywindow.findChild("_found");

_found.addColumn("Line 1",      -1, 1, true, "addr_line1");
_found.addColumn("Line 2",      -1, 1, true, "addr_line2");
_found.addColumn("Line 3",      -1, 1, true, "addr_line3");
_found.addColumn("City",        -1, 1, true, "addr_city");
_found.addColumn("State",       -1, 1, true, "addr_state");
_found.addColumn("Postal Code", -1, 1, true, "addr_postalcode");
_found.addColumn("Country",     -1, 1, true, "addr_country");

function sFillList()
{
  var params = new Object;

  if (_searchMe.line1() != "")
    params.line = _searchMe.line1();
  if (_searchMe.line2() != "")
    params.line = (params.line ? params.line + "|" : "") + _searchMe.line2();
  if (_searchMe.line3() != "")
    params.line = (params.line ? params.line + "|" : "") + _searchMe.line3();
  if (_searchMe.city() != "")
    params.city = _searchMe.city();
  if (_searchMe.state() != "")
    params.state = _searchMe.state();
  if (_searchMe.postalCode() != "")
    params.postal = _searchMe.postalCode();
  if (_searchMe.country() != "")
    params.country = _searchMe.country();

  var qry = toolbox.executeQuery("SELECT * FROM addr "
                     + "WHERE true "
                     + "<? if exists(\"line\") ?>"
                     + " AND (addr_line1 ~* <? value(\"line\") ?>"
                     + "      OR addr_line2 ~* <? value(\"line\") ?>"
                     + "      OR addr_line3 ~* <? value(\"line\") ?>)"
                     + "<? endif ?>"
                     + "<? if exists(\"city\") ?>"
                     + " AND addr_city ~* <? value(\"city\") ?>"
                     + "<? endif ?>"
                     + "<? if exists(\"state\") ?>"
                     + " AND addr_state ~* <? value(\"state\") ?>"
                     + "<? endif ?>"
                     + "<? if exists(\"postal\") ?>"
                     + " AND addr_postalcode ~* <? value(\"postal\") ?>"
                     + "<? endif ?>"
                     + "<? if exists(\"country\") ?>"
                     + " AND addr_country ~* <? value(\"country\") ?>"
                     + "<? endif ?>"
                     + ";",
                     params);
  _found.populate(qry);
}

// open a .ui created as a QDialog
// passing parameters that affect how the QDialog behaves
function sOpenDialog(mode)
{
  // pass NonModal and Window, both of which should be modified by the toolbox for a QDialog
  var childwnd = toolbox.openWindow("addressDialog", mywindow, 0, 1);

  // create a ParameterList to set the address id and whether the dialog
  // should open for editing or viewing
  var params = new Object;
  params.addr_id = _found.id;
  params.mode    = mode;

  // pass the parameters to the dialog. note the use of lastWindow()
  // instead of childwnd. this is because of an internal implementation
  // issue - the QDialog is actually opened inside an XMainWindow and
  // it's that XMainWindow that hosts the scripting environment.
  // XMainWindow checks: if the "addressDialog" script has a "set"
  // property then XMainWindow calls it as a function.
  var tmp = toolbox.lastWindow().set(params);

  // exec() the QDialog
  var execval = childwnd.exec();

  // refill the list when the dialog is done exec()uting
  sFillList();
}

// open a .ui created as a QMainWindow
// passing parameters that affect how the QMainWindow behaves
function sOpenWindow(mode)
{
  // pass NonModal and Window, both of which are honored. you can pass other 
  // values and they should be honored as well.
  var childwnd = toolbox.openWindow("addressWindow", mywindow, 0, 1);

  // create a ParameterList to set the address id and whether the window
  // should open for editing or viewing
  var params = new Object;
  params.addr_id = _found.id;
  params.mode    = mode;

  // unlike the QDialog case, we can pass the parameters directly
  // to the window.
  var tmp = childwnd.set(params);

  // there's no point in calling sFillList because the QMainWindow may
  // remain open for a long time
}

// different slot functions corresponding to the different buttons.
function sEditAddrD()
{
  sOpenDialog("edit");
}

function sEditAddrW()
{
  sOpenWindow("edit");
}

function sViewAddrD()
{
  sOpenDialog("view");
}

function sViewAddrW()
{
  sOpenWindow("view");
}

// open a .ui created as a QWidget
// this could look just like sOpenWindow but has been written differently
// as an example showing that you don't always have to use set() functions
// to something interesting. all of the interesting work in addressCoordinates
// is done via signal/slot connections, so all we have to do here is set
// the id on the AddressCluster widget and let the addressCoordinates
// script and screen do their thing.
function sCoords()
{
  // 0 => NonModal, but you can change that
  var childwnd = toolbox.openWindow("addressCoordinates", mywindow, 0, 3);

  var addr = childwnd.findChild("_addr");
  addr.id = _found.id;

  // there's no point in calling sFillList because the QWidget may
  // remain open for a long time.
}

mywindow.findChild("_query").clicked.connect(sFillList);
mywindow.findChild("_editD").clicked.connect(sEditAddrD);
mywindow.findChild("_editW").clicked.connect(sEditAddrW);
mywindow.findChild("_viewD").clicked.connect(sViewAddrD);
mywindow.findChild("_viewW").clicked.connect(sViewAddrW);
mywindow.findChild("_coords").clicked.connect(sCoords);