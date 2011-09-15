var _addr   = mywindow.findChild("_addr");
var _cancel = mywindow.findChild("_cancel");
var _save   = mywindow.findChild("_save");

function set(input)
{
  if ("addr_id" in input)
  {
    _addr.id = input.addr_id;
    mainwindow.sReportError("_addr.id is now " + _addr.id);
  }
  else
  {
    mainwindow.sReportError("no addr_id passed to set()");
    return 5;
  }

  if ("mode" in input)
  {
    if (input.mode == "view")
    {
      _addr.enabled = false;
      _save.enabled = false;
      _cancel.text  = "Close";
    }
  }

  return 0;
}

function sSave()
{
  var result = _addr.save("CHECK");
  if (result == -2)
  {
    var choice = toolbox.messageBox("question", mywindow, "Conflict",
                                    "<p>This address is shared. Would you like "
                                  + "to save the changes as a new address or "
                                  + "save the changes to all uses of this address?",
                                  0x1000, 0x800);
    if (choice == 0x800)
      result = _addr.save(1);
    else if (choice == 0x1000)
      result = _addr.save(2);                                    
  }
  // not else-if
  if (result < 0)
    toolbox.messageBox("critical", mywindow, "Save Error",
                       "There was an error saving the address");
  else
    mydialog.accept();
}

_save.clicked.connect(sSave);