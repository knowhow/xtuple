//debugger;

var _query         = mywindow.findChild("_query");
var _results       = mywindow.findChild("_results");
var _selector      = mywindow.findChild("_selector");
var _showLineItems = mywindow.findChild("_showLineItems");

_results.addColumn(qsTr("Number"), -1, Qt.AlignLeft, true, "cohead_number");
_results.addColumn(qsTr("Date"),   -1, Qt.AlignLeft, true, "cohead_orderdate");
_results.addColumn(qsTr("Detail"), -1, Qt.AlignLeft, true, "cohead_billtoname");
_results.addColumn(qsTr("Count"),  -1, Qt.AlignRight,true, "counter");

function getParams()
{
  var params = new Object;
  if (_showLineItems.checked)
    params.showItems = true;

  return params;
}

function sQuery()
{
  try {
    var q = toolbox.executeDbQuery("xtreewidgetdemo", _selector.prefix + _selector.value,
                                   getParams());
    _results.populate(q, true);
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, qsTr("Processing Error"), e.message);
  }
}

_query.clicked.connect(sQuery);
