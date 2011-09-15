// create a function that changes the name of a button when called
this.count = 0;
function buttonclicked() {
  this.count += 1;
  mywindow.findChild("_new").text="New" + this.count;
}

// connect up so when the _edit object is clicked is calls the function
mywindow.findChild("_new").clicked.connect(buttonclicked);
