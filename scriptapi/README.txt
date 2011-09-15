This directory holds the interface between various classes and the
scripting engine available to the xTuple ERP GUI client.

There are several ways to use a class while scripting:
- Use a Qt class as is - this restricts you to using the
  properties, slots, and signals as defined by Qt. This only works for
  classes which inherit from QObject.
- Extend a QObject subclass - make more of its methods Q_INVOKABLE.
- create a prototype class.
- as a last resort, add to the ScriptToolbox

Using Qt classes as they are:
-----------------------------
[ not written yet ]

Extending QObject subclasses:
-----------------------------
If the class is part of the xTuple ERP sources and inherits from QObject
then you should do the following:
- make the public methods in your class Q_INVOKABLE
- add Q_INVOKABLE methods from the parent classes if they are not
  already exposed to scripting
- write a constructor function
- write conversion functions XtoScriptValue and XfromScriptValue and
  register these conversion functions with qScriptRegisterMetaType
  If all you need to do is pass pointers back and forth, you can write these
  like so:
  QScriptValue XtoScriptValue(QScriptEngine *engine, X* const &obj)
  {
    return engine->newQObject(obj);
  }
  QScriptValue XfromScriptValue(const QScriptValue obj X* &item)
  {
    item = qobject_cast<X*>(obj.toQObject());
  }
  void setupX(QScriptEngine *engine)
  {
    qScriptRegisterMetaType(engine, XtoScriptValue, XfromScriptValue);

    // register the constructor function if you have one
  }
- add this setup function and Q_DECLARE_METATYPE(X*) to the appropriate header
- expose any enumerations used
- call this setup function
  If the class is defined in common or widgets directories, add the call to
  scriptapi/setupscriptapi.cpp. Otherwise add it to guiclient.cpp?

you need to use in the appropriate header file and recompile. The
following method is for classes you /don't/ have control over, such
as Qt classes themselves.

Short Description of creating a Prototype class:
$ edit newproto.h
$ awk -f h2cpp.awk newproto.h > newproto.cpp
$ edit newproto.cpp
$ edit scriptapi.pro      # add newproto.h and newproto.cpp
$ edit setupscriptapi.cpp # add #include "newproto.h" and setupNewProto(engine)

Long Description:
To add a prototype for the class named 'new', you have to create header
and implementation files, add these two files to the scriptapi project,
and register the new prototype class with the scripting engine.

$ edit newproto.h
  Create the header file. It's probably best to start with a copy
  of an existing small prototype .h. Change the #ifndef, #define,
  #includes, Q_DECLARE_METATYPE, setup* and construct* function
  names, and the class name and constructor at the top to refer to
  your new class.  Replace the Q_INVOKABLE methods of the copied
  file with signatures for the public methods in the class you want
  to expose. One way to get a complete list is to copy the class
  summary from the documentation page, paste it into your editor,
  and add Q_INVOKABLE.  You will probably have to replace symbolic
  constants with integer literals and references to enums with int.

$ awk -f h2cpp.awk newproto.h > newproto.cpp
  This awk script takes the edited header file and fills in the
  boilerplate code necessary to make most methods work. The awk
  script is really dumb, so:

$ edit newproto.cpp
  Edit the resulting C++ implementation by hand. You'll have to do
  things like move * from after NewProto:: to before it 
  remove data type names from function calls, remove
  const from the end of function call lines, fill in the occasional
  variable names where method parameters in the .h were declared
  with a type but no name.  Methods that return pointers will need
  their default return values changed to 0 or NULL.
  
  For example, when creating qtabwidgetproto.cpp, the following
  code was generated:
    QWidget QTabWidgetProto::*cornerWidget(int corner) const
    {
      QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
      if (item)
        return item->*cornerWidget(int corner) const;
      return QWidget();
    }

  This had to be changed to:
    QWidget *QTabWidgetProto::cornerWidget(int corner) const
    {
      QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
      if (item)
        return item->cornerWidget(corner);
      return 0;
    }

  If there's a reasonable QString representation of an object,
  modify the default implementation of the toString() method
  to reflect this and add toString() to the .h file. Otherwise
  remove or comment out the default toString().

$ edit scriptapi.pro      # add newproto.h and newproto.cpp
$ edit setupscriptapi.cpp # add #include "newproto.h" and setupNewProto(engine)
