function expandClicked()
{
mywindow.findChild("_soitem").collapseAll();
}

var btnView = mywindow.findChild("_soitem");
var layout = toolbox.widgetGetLayout(btnView);
var newbutton = toolbox.createWidget("QPushButton", mywindow, "_expandButton");
newbutton.text="C&ollapse All";
toolbox.layoutBoxInsertWidget(layout, 3, newbutton);
newbutton.clicked.connect(expandClicked);
