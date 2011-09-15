function identifierToTag(identifier, namespace)
{
  try {
    if (namespace instanceof Array)
    {
      for (var i in namespace)
      {
        var nsidx = identifier.indexOf(namespace[i].ns + "_");
        if (nsidx == 0)
          return namespace[i].ns + ":" +
                 identifier.substring(namespace[i].ns.length + 1);
      }
      return null;
    }
    else if (namespace instanceof Object)
    {
      var nsidx = identifier.indexOf(namespace.ns + "_");
        if (nsidx == 0)
          return namespace.ns + ":" +
                 identifier.substring(namespace.ns.length + 1);
      return null;
    }
    else if (namespace != null)
    {
      var nsidx = identifier.indexOf(namespace + "_");
      if (nsidx == 0)
        return namespace + ":" + identifier.substring(namespace.length + 1);
      return null;
    }
    else
      return identifier;
  }
  catch(e) {
    print("identifierToTag exception at line " + e.lineNumber + ": " + e);
  }
}

function js2xml(input, inputname, namespace, level)
{
  try {
  if (input == null)
    return "";

  if (level == null)
    level = 0;

  var tag = identifierToTag(input.xmltag == null ? inputname : input.xmltag,
                            namespace);
  if (tag == null || tag.length == 0)
    return "";

  var indent = "";
  for (var i = 0; i < level; i++)
    indent += "  ";

  var output = "";

  
  var xmlattribute = "";
  if (input.xmlattribute != null)
    for (var i in input.xmlattribute)
      xmlattribute += ' ' + i + '="' + input.xmlattribute[i] + '"';

  if (! hasXMLChildren(input, namespace))
  {
    output += indent + "<" + tag + xmlattribute;
    if ((input + "").indexOf("[") != 0)
      output += ">" + input + "</" + tag + ">\n";
    else
      output += "/>\n"
  }
  else
  {
    if (input instanceof Array)
    {
      for (var i = 0; i < input.length; i++)
        output += js2xml(input[i], input[i].xmltag, namespace, level + 1);
    }
    else if (typeof input == "object")
    {
      output += indent + "<" + tag + xmlattribute + ">\n";
      for (var j in input)
        output += js2xml(input[j], j, namespace, level + 1);
      if (input.xmltext != null)
        output += indent + "  " + input.xmltext + "\n";
      output += indent + "</" + tag + ">\n";
    }
  }

  return output;
  }
  catch(e) {
    print("js2xml exception at line " + e.lineNumber + ": " + e);
  }
}

function hasXMLChildren(object, namespace)
{
  try {
    if (object.xmltext != null)
      return true;

    else if (namespace instanceof Array)
    {
      for (var i in namespace)
        if (hasXMLChildren(object, namespace[i]))
          return true;
    }
    else if (namespace instanceof Object)
      return hasXMLChildren(object, namespace.ns);
    else if (namespace != null)
    {
      if (object instanceof Array)
      { 
        if (object["xmltag"].indexOf(namespace + "_") == 0)
          return true;
      }
      else if (object instanceof Object)
      { 
        for (var j in object)
        { 
          if ((j + "").indexOf(namespace + "_") == 0)
            return true;
        }
      }
    }
    else
    {
      print("hasXMLChildren: namespace is empty!");
      if (object instanceof Array)
        return true;
      else if (object instanceof Object)
        return true;
    }

    return false;
  }
  catch(e) {
    print("hasXMLChildren exception at line " + e.lineNumber + ": " + e);
  }
}

function js2soap(input, namespace) {
  try {
  var output = "";
  var nsprefix = '                   xmlns:';
  output += '<?xml version="1.0" encoding="UTF-8"?>\n';
  output += '<SOAP-ENV:Envelope\n';
  output += nsprefix + 'SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"\n';
  output += nsprefix + 'SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/"\n';
  output += nsprefix + 'xsi="http://www.w3.org/2001/XMLSchema-instance"\n';
  output += nsprefix + 'xsd="http://www.w3.org/2001/XMLSchema"\n';
  if (namespace instanceof Array)
    for (var i in namespace)
      output += nsprefix + namespace[i].ns + '="' + namespace[i].url + '"\n';
  else if (namespace instanceof Object)
    output += nsprefix + namespace.ns + '="' + namespace.url + '"\n';

  output += '>\n';
  output += "  <SOAP-ENV:Body>\n";

  output += js2xml(input, input.xmltag, namespace, 2);

  output += "  </SOAP-ENV:Body>\n";
  output += "</SOAP-ENV:Envelope>";
  
  return output;
  }
  catch (e) {
    print("js2soap exception at line " + e.lineNumber + ": " + e);
  }
}