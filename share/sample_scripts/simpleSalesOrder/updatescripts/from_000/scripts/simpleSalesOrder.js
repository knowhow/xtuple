// Define local variables
var _linenumCol = 1;
var _itemCol    = 2;
var _statusCol  = 6;	
var _qtyCol     = 7;
var _priceCol   = 9;

var _billAddrNo = "";
var _linenumber = 1;
var _populating = false;
var _extList = new Array();
var _taxList = new Array();

var _newMode    = 0;
var _editMode   = 1;
var _viewMode   = 2;

var _add		= mywindow.findChild("_add");
var _address	= mywindow.findChild("_address");
var _cancel		= mywindow.findChild("_cancel");
var _cust      	= mywindow.findChild("_cust");
var _custTab	= mywindow.findChild("_custTab");
var _extendedPrice	= mywindow.findChild("_extendedPrice");
var _item		= mywindow.findChild("_item");
var _itemGroup    	= mywindow.findChild("_itemGroup");
var _itemsTab	= mywindow.findChild("_itemsTab");
var _number		= mywindow.findChild("_number");
var _qty		= mywindow.findChild("_qty");
var _remove		= mywindow.findChild("_remove");
var _sale		= mywindow.findChild("_sale");
var _saleitem	= mywindow.findChild("_saleitem");
var _saleitems	= mywindow.findChild("_saleitems");
var _salesrep	= mywindow.findChild("_salesrep");
var _save		= mywindow.findChild("_save");
var _scheddate	= mywindow.findChild("_scheddate");
var _shipto		= mywindow.findChild("_shipto");
var _shiptoGroup	= mywindow.findChild("_shiptoGroup");
var _shipvia	= mywindow.findChild("_shipvia")
var _site		= mywindow.findChild("_site");
var _siteLit	= mywindow.findChild("_siteLit");
var _subtotal   	= mywindow.findChild("_subtotal");
var _tab		= mywindow.findChild("_tab");
var _tax		= mywindow.findChild("_tax");
var _taxauth	= mywindow.findChild("_taxauth");
var _total		= mywindow.findChild("_total");	
var _unitPrice  	= mywindow.findChild("_unitPrice");

// Set Columns
with (_saleitems)
{
  setColumn("Line#"  		, 40,  0, true,  "line_number");
  setColumn("Item"   		, -1,  0, true,  "item_number");
  setColumn("Customer P/N"   	, 100, 0, false, "customer_pn");
  setColumn("Substitute for"   	, 100, 0, false, "substitute_for");
  setColumn("Site"   		, 40,  0, false, "sold_from_site");
  setColumn("Status"   		, 40,  0, false, "status");
  setColumn("Quantity"		, 100, 0, true,  "qty_ordered");
  setColumn("Qty UOM"   	, 40,  0, false, "qty_uom");
  setColumn("Price" 		, 100, 0, true,  "net_unit_price");
  setColumn("Price UOM"   	, 40,  0, false, "price_uom");
  setColumn("Extended"		, 100, 0, true,  "extension");
  setColumn("Sched. Date"	, 100, 0, true,  "scheduled_date");
  setColumn("Promise Date"	, 80,  0, false, "promise_date");
  setColumn("Warranty"   	, 40,  0, false, "warranty");
  setColumn("Tax Type"   	, 80,  0, false, "tax_type");
  setColumn("Tax Code"   	, 80,  0, false, "tax_code");
  setColumn("Discount"   	, 40,  0, false, "discount_pct_from_list");
  setColumn("Create Order"   	, 40,  0, false, "create_order");
  setColumn("Overwrite P/O"   	, 40,  0, false, "overwrite_po_price");
  setColumn("Notes"   		,100,  0, false, "notes");
  setColumn("Alt. COS"   	,100,  0, false, "alternate_cos_account");
}

// Define connections
_add.clicked.connect(add);
_cancel.clicked.connect(cancel);
_cust["newId(int)"].connect(handleButtons);
_cust["newId(int)"].connect(populateCustomer);
_item["newId(int)"].connect(itemCheck);
_item["newId(int)"].connect(handleButtons);
_item["newId(int)"].connect(itemPrice);
_item["valid(bool)"].connect(_add["setEnabled(bool)"]);
_qty["textChanged(const QString&)"].connect(itemPrice);
_qty["textChanged(const QString&)"].connect(extension);
_remove.clicked.connect(remove);
_saleitems["rowSelected(int)"].connect(rowSelected);
_saleitems["valid(bool)"].connect(_remove["setEnabled(bool)"]);
_salesrep["newID(int)"].connect(handleButtons);
_save.clicked.connect(save);
_taxauth["newID(int)"].connect(handleButtons);
_unitPrice.valueChanged.connect(extension);

// Misc Defaults
_itemGroup.enabled = false;
_qty.setValidator(toolbox.qtyVal());
_item.setType(ItemLineEdit.cSold | 
	ItemLineEdit.cItemActive |
	ItemLineEdit.cPurchased ^ 
	ItemLineEdit.cManufactured ^  
	ItemLineEdit.cReference);
_site.setVisible(metrics.value("MultiWhs") == "t");
_siteLit.setVisible(metrics.value("MultiWhs") == "t");
handleButtons();

// Define local functions
function add()
{
  _saleitems.insert();
  _saleitem.clear();
  _saleitems.setValue(_saleitem.currentIndex(),_linenumCol,_linenumber);
  _itemGroup.enabled = true;
  _linenumber = _linenumber + 1;
  _item.setFocus();
}

function cancel()
{
  if (_sale.mode == _newMode)
  {
    params = new Object;
    params.number = _number.text;
    toolbox.executeDbQuery("simplesalesorder","releasesonumber",params);
    mydialog.accept();
  }
  else
    mydialog.reject();
}

function extension()
{
  var ext = _qty.text * _unitPrice.localValue;
  _extendedPrice.setLocalValue(ext)

  if (_populating)  // No need to keep recalculating totals
    return;

  try
  {
    var row = _saleitem.currentIndex();
    var tax;

    // Remove original values from total
    _subtotal.localValue = _subtotal.localValue -_extList[row];
    _tax.localValue = _tax.localValue -_taxList[row];

    // Update item extension
    _extList[row] = ext;
    _subtotal.localValue = _subtotal.localValue + ext;

    // Recalculate tax
    var params = new Object;
    params.taxauth_id = _taxauth.id();
    params.item_number = _item.itemNumber();
    params.qty = _qty.text - 0;
    params.unit_price = _unitPrice.localValue - 0;
  
    var data = toolbox.executeDbQuery("simplesalesorder","itemtax",params);
    if (data.first())
      tax = data.value("item_tax") - 0;
    else
      tax = 0 - 0;
    _taxList[row] = tax;
    _tax.localValue = _tax.localValue + tax;

    // Update total
    _total.localValue = _subtotal.localValue + _tax.localValue;
  }
  catch (e)
  {
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  } 
}

function handleButtons()
{
  var state = (_cust.id() != -1 && 
               _salesrep.id() != -1 &&
               _taxauth.id() != -1 &&
              (_saleitems.rowCountVisible() > 1 ||
               _item.number.length))
  _save.enabled = (state);
  _site.enabled = (!_saleitems.rowCountVisible() || !_item.isValid());
  _taxauth.enabled = (!_saleitems.rowCountVisible() || !_item.isValid());
  toolbox.tabSetTabEnabled(_tab, 
		toolbox.tabTabIndex(_tab,_itemsTab),
		_taxauth.id() != -1)
}

function itemCheck()
{
  if (_item.id() == -1)
    return;

  var params = new Object;
  params.item_id = _item.id();
  params.warehous_id = _site.id();
  
  data = toolbox.executeDbQuery("simplesalesorder","itemcheck",params);
  if (data.first())
    if (data.value("result"))
      return;
 
  var msg = "Selected item is invalid at site " + _site.text + ".";
  toolbox.messageBox("critical", mywindow, mywindow.windowTitle, msg);
  _item.setId(-1);
  _item.setFocus();
}

function itemPrice()
{
  if (_populating) // Don't overwrite data stored in the db being populated
    return;

  var params = new Object;
  params.item_id = _item.id();
  params.cust_id = _cust.id();
  params.qty     = _qty.text;

  try
  {
    if (_qty.text && _item.id() > 0)
    {
      var data = toolbox.executeDbQuery("simplesalesorder","itemprice",params);
      if (data.first())
        _unitPrice.setLocalValue(data.value("itemprice"));
      else if (data.lastError())
        throw data.lastError().text;
      else
        throw "Price not found.  Please see your administrator.";
    }
  }
  catch (e)
  {
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  }
}

function populate()
{
  _populating = true;
  _sale.select();

  populateItems();

  if (_saleitems.rowCount())
  {
    _saleitem.setCurrentIndex(0);
    _saleitems.selectRow(0);
  }

  _populating = false;
  handleButtons();
  _cust.enabled = false;
}

function populateCustomer()
{
  var params = new Object;
  params.cust_id = _cust.id();

  var data = toolbox.executeDbQuery("simplesalesorder","fetchcustomer", params);
  if(data.first())
  {
    _shiptoGroup.enabled = data.value("cust_ffshipto");
    if (_populating)
      return;
    _address.setNumber(data.value("address_number"));
    _taxauth.code = data.value("taxauth");
    _salesrep.code = data.value("salesrep");
    _shipvia.code = data.value("shipvia");
    _site.code = data.value("site");
  }
  else if (_populating)
    return;
  else
  {
    _address.clear();
    _salesrep.code = "";
    _taxauth.code = "";
    _shipvia.code = "";
  }
}

function populateItems()
{
  _saleitems.populate(_sale.currentIndex());
  _itemGroup.enabled = (_saleitems.rowCount());
  _item.enabled = (!_saleitems.rowCount());
  if (_saleitems.rowCount())
    _linenumber = _saleitems.value(_saleitems.rowCount()-1,_linenumCol) - 0 + 1;
  else
    _linenumber = 1;
  recalcTotals();
}

function prepare()
{
  // Set header data
  _address.clear();
  var data = toolbox.executeDbQuery("simplesalesorder", "fetchsonumber");
  if(data.first())
    _number.text = data.value("number");

  // Associate sale items to new header
  populateItems();
  add();
  _add.enabled = false;
  _tab.setCurrentIndex(0);
  _cust.setFocus();
}

function recalcTotals()
{
  var params = new Object;
  var data;
  var ext = 0-0;
  var tax = 0-0;
  _subtotal.clear();
  _tax.clear();
  _total.clear();
  _extList = [];
  _taxList = [];
  for (var i = 0 - 0; i < _saleitems.rowCount(); i++)
  {
    ext = _saleitems.value(i,_qtyCol) * 
          _saleitems.value(i,_priceCol);
    _extList[i] = ext;
    _subtotal.localValue = _subtotal.localValue + ext;

    params.taxauth_id = _taxauth.id();
    params.item_number = _saleitems.value(i,_itemCol);
    params.qty = _saleitems.value(i,_qtyCol) - 0;
    params.unit_price = _saleitems.value(i,_priceCol) - 0;
    data = toolbox.executeDbQuery("simplesalesorder","itemtax",params);
    if (data.first())
      tax = data.value("item_tax") - 0;
    else
      tax = 0 - 0;
    _taxList[i] = tax;
    _tax.localValue = _tax.localValue + tax;
  }
  _total.localValue = _subtotal.localValue + _tax.localValue;
}

function remove()
{
  var num = _saleitems.selectedValue(_linenumCol);
  var row = _saleitem.currentIndex();
  var idx = 0;
  var params = new Object;

  // Make sure we can do this
  params.number = _number.text;
  params.line_number = _saleitems.selectedValue(_linenumCol);
  data = toolbox.executeDbQuery("simplesalesorder","removecheck",params);
  if (data.first())
  {
    if (!data.value("result"))
    {
      msg = "Line item has shipping transaction activity and may not be removed."
      toolbox.messageBox("critical", mywindow, mywindow.windowTitle, msg);
      return;
    }
  }

  _itemGroup.enabled = false;
  _saleitem.clear();
  _saleitems.removeSelected();

  // Select a valid row, otherwise enable/disable appropriate controls
  if (!_saleitems.rowCountVisible())
  {
    _itemGroup.enabled = false;
    _add.enabled = true;
    _remove.enabled = false;
    handleButtons();
    return;
  }
  else
  {
    while (_saleitems.isRowHidden(idx))
      idx++;
    _populating = true;
    _saleitem.setCurrentIndex(idx);
    _saleitems.selectRow(idx);
    popualting = false;
  }
}

function rowSelected(row)
{
  try
  {
    var currentRow = _saleitem.currentIndex(row);
    if (row == currentRow)
      return;

    if (_itemGroup.enabled)
    {
      if (_item.isValid() == false)
        throw "You must select an item or remove the current line."
      else if (!_scheddate.date.toString().length)
        throw "You must enter a valid scheduled date or remove the current line."
      else if (!_qty.text.length)
        throw "You must enter a valid quantity or remove the current line."
    }
 
    _populating = true;
    _saleitem.setCurrentIndex(row);
    _item.enabled = !_saleitems.value(row,_statusCol).length;
    _itemGroup.enabled = (_saleitems.value(row,_statusCol) != "X");
    _populating = false;
  }
  catch (e)
  {
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
    _saleitems.selectRow(currentRow);
  }
}

function save()
{
  if (_item.id() == -1)
    remove();

  _populating = true;
  _address.check();

  try
  {
    toolbox.executeBegin();
    _sale.save();
    _saleitem.save();
    toolbox.executeCommit();
    if (_sale.mode == _newMode)
      prepare();
    else
      mydialog.accept();
  }
  catch (e)
  {
    toolbox.executeRollback();
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  }
  finally
  {
    _populating = false;
  }
}

function set(input)
{
  if ("mode" in input)
  {
    _sale.setMode(input.mode);

    if (input.mode == _newMode)
      prepare();
    else if (input.mode == _viewMode)
    {
       _save.hide();
       _cancel.text = "Close";
       _add.hide();
       _remove.hide(); 
       _itemGroup.hide();
    }

    // Always disabled
    _number.enabled 	= false;
  }

  if ("filter" in input) // Use to populate an existing record
  {
    _sale.setFilter(input.filter);
    populate();
  }

  return 0;
}
