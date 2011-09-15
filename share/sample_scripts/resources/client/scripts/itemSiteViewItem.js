/*
  This is a sample script showing how to open core application windows
  from scripts, setting up and populating XTreeWidgets, and
  connecting signals from Qt widgets to scripted slots,

  This sample creates a window that listing the itemsites for a selected site.
  One QPushButton lets the user view the Item record for the selected
  row while another lets the user view the Item Site record.
  The Item window is defined as an XWidget.
  The Item Site window is an XDialog, and so has to be handled differently.
 */

var _itemsite = mywindow.findChild("_itemsite");
var _site     = mywindow.findChild("_site");
var _view     = mywindow.findChild("_view");
var _viewis   = mywindow.findChild("_viewItemSite");

_itemsite.addColumn(qsTr("Item Number"),     -1, Qt.AlignLeft, true, "item_number");
_itemsite.addColumn(qsTr("Loc. Controlled"), -1, Qt.AlignRight,true, "itemsite_loccntrl");
_itemsite.addColumn(qsTr("Control Method"),  -1, Qt.AlignRight,true, "itemsite_controlmethod");

function sFillList()
{
  var params = new Object;
  params.site = _site.id();

  var qry = toolbox.executeQuery('SELECT item_id, itemsite_id, item_number, '
                               + '       itemsite_loccntrl, itemsite_controlmethod '
                               + 'FROM itemsite '
                               + '     JOIN item ON (itemsite_item_id=item_id) '
                               + 'WHERE itemsite_warehous_id=<? value("site") ?> '
                               + 'ORDER BY item_number;',
                               params);
  _itemsite.populate(qry, true);
}

function sViewItem()
{
  var params = new Object();
  params.item_id = _itemsite.id();
  params.mode    = "view";

  // here's the interesting line:    
  var newdlg = toolbox.openWindow("item", 0, 0, 0);
  /*                               ^      ^  ^  ^- window flag
                                   |      |  +- modality
                                   |      +- parent
                                   +- open an instance of this class
   The "item" class is defined as a QWidget (look at item.ui). If we set the
   parent to mywindow then the item window will appear as a widget within
   mywindow, which we don't want. Therefore set the parent to 0, which means
   "no parent". If item.ui were a QMainWindow or QDialog then we could
   set the parent to mywindow and everything would work out OK. In fact,
   if it were a QDialog then we could set the modality to Qt.ApplicationModal
   to block all other input or Qt.WindowModal to block input to mywindow
   until the Item window were closed.
  */
 
  newdlg.set(params);
}

function sViewItemSite()
{
  var params = new Object();
  params.itemsite_id= _itemsite.altId();
  params.mode       = "view";

  // here's one interesting line for opening a QDialog:    
  var newdlg = toolbox.openWindow("itemSite", mywindow, Qt.WindowModal, Qt.Sheet);
  /*                               ^          ^         ^               ^- window flag
                                   |          |         +- modality
                                   |          +- parent
                                   +- open an instance of this class
   The "itemsite" class is defined as a QDialog (look at itemsite.ui). By
   setting the parent to mywindow we're saying that the itemsite window
   should be cleaned up when mywindow eventually gets closed. We're also
   saying how the Window modality should work: block input to mywindow
   until the Item Site window gets closed.
  */
 
  newdlg.set(params);

  // QDialog objects have the exec() method, so we can capture the result
  var result = newdlg.exec();
  print("Item Site window returned " + result); // here we send it to stderr
}

_site["newID(int)"].connect(sFillList);
_view.clicked.connect(sViewItem);
_viewis.clicked.connect(sViewItemSite);

sFillList();
