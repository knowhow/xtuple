function expandClicked()
{
mywindow.findChild("_soitem").expandAll();
}

var btnView = mywindow.findChild("_soitem");
var layout = toolbox.widgetGetLayout(btnView);
var newbutton = toolbox.createWidget("QPushButton", mywindow, "_expandButton");
newbutton.text="E&xpand All";
toolbox.layoutBoxInsertWidget(layout, 2, newbutton);
newbutton.clicked.connect(expandClicked);

