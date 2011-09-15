// Script: postOperations
// This script will prevent users from posting more qty than the balance
// remaining on the selected operation. This takes advantage of the
// _balance dynamic property set by the postOperations screen as of 3.0.0RC
function checkQty()
{
  if(mywindow.findChild("_qty").text > mywindow._balance)
  {
    mywindow.findChild("_post").enabled=false;
    toolbox.messageBox("error", mywindow, "Cannot Over Post",
      "You may not over post qty. Please correct this situation.");
  }
  else
    mywindow.findChild("_post").enabled=true;
}

var qty = mywindow.findChild("_qty");
qty.lostFocus.connect(checkQty);
