// two ways to do this
// first you can reference the object name directly
// but this has the disadvantage of either not knowing
// where in the layout an object is or that the object
// can be moved.
mywindow.widget._new.enabled=false;

// the other way is to use findChild() which is more portable and maintainable
mywindow.findChild("_new").visible = false;
