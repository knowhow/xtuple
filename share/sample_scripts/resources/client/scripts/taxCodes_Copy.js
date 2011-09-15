function copyClicked()
{
  var params = new Object;
  params.tax_id = mywindow.findChild("_tax").id();

  var qry = toolbox.executeQuery("INSERT INTO tax"
                                +"      (tax_code, tax_descrip,"
                                +"       tax_sales_accnt_id, tax_ratea,"
                                +"       tax_salesb_accnt_id, tax_rateb,"
                                +"       tax_salesc_accnt_id, tax_ratec,"
                                +"       tax_freight, tax_cumulative) "
                                +"SELECT tax_code||'_copy', tax_descrip,"
                                +"       tax_sales_accnt_id, tax_ratea,"
                                +"       tax_salesb_accnt_id, tax_rateb,"
                                +"       tax_salesc_accnt_id, tax_ratec,"
                                +"       tax_freight, tax_cumulative"
                                +"  FROM tax"
                                +" WHERE(tax_id=<? value(\"tax_id\") ?>);", params);
  if(qry.numRowsAffected()<1)
  {
    toolbox.messageBox("information", mywindow,
      "Copy Failed", "There was an error copying the selected Tax Code. "
     +"Please check the Database Logs for additional information.");
    return;
  }

  mywindow.sFillList();
}

var btnView = mywindow.findChild("_view");
var layout = toolbox.widgetGetLayout(btnView);
var newbutton = toolbox.createWidget("QPushButton", mywindow, "_copyButton");
newbutton.text="Cop&y";
toolbox.layoutBoxInsertWidget(layout, 3, newbutton);
newbutton.clicked.connect(copyClicked);
newbutton.enabled=false;
var taxlist = mywindow.findChild("_tax");
taxlist.valid.connect(newbutton, "setEnabled");
