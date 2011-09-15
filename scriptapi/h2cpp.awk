# do the bulk of the plug-n-chug creation of a prototype class .cpp from a
# prototype .h. you decide what should be exposed and this script creates
# most of the boilerplate code.  the resulting .cpp must still be edited by
# hand.

/^\/\*/,/^ *\*\// {
  print;
  if ($1 ~ /\*\//)
  {
    print "";
    print "#include \"" FILENAME "\"";
    print "";
  }

  next;
}

/^class  *[A-Za-z][A-Za-z0-9_]*;/ {
  sub(/;/, "", $2);
  print "#include <" $2 ">";
}

/^void setup.*QScriptEngine/ {
  protoclass = substr($2, 6, index($2, "(") - 6);
  baseclass  = substr(protoclass, 1, index(protoclass, "Proto") - 1);
  print "QScriptValue " baseclass "toScriptValue(QScriptEngine *engine, " baseclass "* const &item)";
  print "{ return engine->newQObject(item); }";
  print "";
  print "void " baseclass "fromScriptValue(const QScriptValue &obj, " baseclass "* &item)";
  print "{\n  item = qobject_cast<" baseclass "*>(obj.toQObject());\n}";
  print "";
  sub(/;/, "");
  print;
  print "{";
  print " qScriptRegisterMetaType(engine, " baseclass "toScriptValue, " baseclass "fromScriptValue);"
  print "";
  print "  QScriptValue proto = engine->newQObject(new " protoclass "(engine));";
  print "  engine->setDefaultPrototype(qMetaTypeId<" baseclass "*>(), proto);";
  print "  engine->setDefaultPrototype(qMetaTypeId<" baseclass ">(),  proto);";
  print "";
  print "  QScriptValue constructor = engine->newFunction(construct" baseclass ",";
  print "                                                 proto);";
  print "  engine->globalObject().setProperty(\"" baseclass "\",  constructor);";
  print "}";
  print "";
}

/^QScriptValue construct/ {
  print "QScriptValue construct" baseclass "(QScriptContext * /*context*/,";
  print "                                    QScriptEngine  *engine)";
  print "{";
  print "  " baseclass " *obj = 0;";
  print "  /* if (context->argumentCount() ...)";
  print "  else if (something bad)";
  print "    context->throwError(QScriptContext::UnknownError,";
  print "                        \"Could not find an appropriate " baseclass "constructor\");";
  print "  else";
  print "  */";
  print "    obj = new " baseclass "();";
  print "  return engine->toScriptValue(obj);";
  print "}";
  print ""
}

$1 ~ protoclass ".QObject" {
  sub(/^  */, "");
  sub(/;/,    "");
  print protoclass "::" $0;
  print "    : QObject(parent)"
  print "{\n}\n";
}

/Q_INVOKABLE/ {
  sub(/ *Q_INVOKABLE */, "");
  sub(/  */, " ");
  sub(/;/,   "");
  type    = $1;
  functn  = substr($2, 1, index($2, "(") - 1);
  arglist = substr($0, index($0, "("), length - index($0, "(") + 1);
  gsub(/ = [^,]*(\(\))?\)/, ")", arglist);
  gsub(/ = [^,]*(\(\))?,/, ",", arglist);
  print $1, protoclass "::" functn arglist
  print "{";
  print "  " baseclass " *item = qscriptvalue_cast<" baseclass "*>(thisObject());";
  print "  if (item)";
  if (type == "void")
  {
    print "    item->" functn arglist ";";
  }
  else
  {
    print "    return item->" functn arglist ";";
    if (type == "int")         print "  return 0;";
    else if (type == "qint64") print "  return 0;";
    else if (type == "uint")   print "  return 0;";
    else if (type == "bool")   print "  return false;";
    else                       print "  return " type "();";
  }
  print "}";
  print "";
  if (functn == "toString")
    foundToString = 1;
}

END {
  if (! foundToString)
  {
    print "QString " protoclass "::toString() const";
    print "{";
    print "  " baseclass " *item = qscriptvalue_cast<" baseclass "*>(thisObject());";
    print "  if (item)";
    print "    return QString(\"" baseclass "()\");";
    print "  return QString(\"" baseclass "(unknown)\");";
    print "}";
  }
}
