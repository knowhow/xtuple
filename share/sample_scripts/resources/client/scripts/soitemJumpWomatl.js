function womatlClicked()
{
  var params = new Object;
  params.sonumber = mywindow.findChild("_orderNumber").text;
  params.linenumber = mywindow.findChild("_lineNumber").text;

  var qry = toolbox.executeQuery("SELECT wo_id"
                                +"  FROM wo, cohead, coitem"
                                +" WHERE((wo_ordtype='S')"
                                +"   AND (wo_ordid=coitem_id)"
                                +"   AND (coitem_cohead_id=cohead_id)"
                                +"   AND (coitem_linenumber=<? value(\"linenumber\") ?>)"
                                +"   AND (cohead_number=<? value(\"sonumber\") ?>));", params);
  if(!qry.first())
  {
    toolbox.messageBox("information", mywindow, "No W/O Available", "There currently is no W/O available for displaying.");
    return;
  }
  
  params = new Object;
  params.wo_id = qry.value("wo_id");
  params.mode = "edit";

  var act = mainwindow.findChild("wo.maintainWoMaterialRequirements");
  if(act != null)
  {
    act.trigger();
  
    var wnd = toolbox.lastWindow();
    wnd.set(params);
  }
}

var btnCancel = mywindow.findChild("_cancel");
var layout = toolbox.widgetGetLayout(btnCancel);
var newbutton = toolbox.createWidget("QPushButton", mywindow, "_womatlButton");
newbutton.text="W/O Matl.";
toolbox.layoutBoxInsertWidget(layout, 4, newbutton);
newbutton.clicked.connect(womatlClicked);
