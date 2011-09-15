function tagToIdentifier(tag)
{
  if (tag == "#text")
    return "xmltext";

  var nsidx = tag.indexOf(":");
  if (nsidx > 0)
    return tag.substring(0, nsidx) + "_" + tag.substring(nsidx + 1);
  else
    return tag;
}

function xml2js(input)
{
  try {
    if ((input + "").indexOf("QDom") != 0 &&
        (input + "").indexOf("[QDom") != 0)
    {
      var doc  = new QDomDocument();
      var err  = new Object();
      var line = 0;
      var col  = 0;
      if (! doc.setContent(input, false, err, line, col))
        throw "error reading XML at " + line + ":" + col + "\n" + err;
      input = doc;
    }

    var attribute = input.attributes();
    var nodename = tagToIdentifier(input.nodeName());
    var xmlattribute;
    var xmltag = nodename;

    if (attribute.size() > 0)
    {
      xmlattribute = new Object();
      for (var attr = 0; attr < attribute.size(); attr++)
        xmlattribute[tagToIdentifier(attribute.item(attr).toAttr().name())] =
                                    attribute.item(attr).toAttr().value();
    }

    var child = input.childNodes();
    var seq = 0;
    var output = new Object();

    for (var i = 0; i < child.size(); i++)
    {
      var childtag = tagToIdentifier(child.at(i).nodeName());
      if (child.at(i).isText())
      {
        output = new String(child.at(i).nodeValue().split(/\s+/).join(" "));
        output.xmltag = xmltag;
        seq = 0;
      }
      else
      if (child.size() > i + 1 &&
          child.at(i).nodeName() == child.at(i + 1).nodeName())
      {
        if (output[childtag] == null)
        {
          output[childtag] = new Array();
          output[childtag].xmltag = childtag;
        }
        output[childtag][seq] = xml2js(child.at(i));
        if (output[childtag][seq])
          output[childtag][seq].xmltag = childtag;
        seq = seq + 1;
      }
      else if (child.size() > 1 &&
               i > 0 &&
               child.at(i).nodeName() == child.at(i - 1).nodeName())
      {
        output[childtag][seq] = xml2js(child.at(i));
        if (output[childtag][seq])
          output[childtag][seq].xmltag = childtag;
        seq = seq + 1;
      }
      else
      {
        output[childtag] = xml2js(child.at(i));
        if (output[childtag]) {
          output[childtag].xmltag = childtag; }
        seq = 0;
      }
    }

    output.xmltag = xmltag;
    if (xmlattribute != null)
      output.xmlattribute = xmlattribute;
    return output;
  }
  catch (e) {
    print("xml2js - exception at line " + e.lineNumber + ": " + e);
  }
}

/* take a SOAP document in the form of a string and return a javascript
   object with the contents of the SOAP document as its properties.
   if the SOAP-ENV:Body contains multiple elements, the object returned
   will be an array of JS objects
*/
function soap2js(input)
{
 try {
    var doc  = new QDomDocument();
    var err  = new Object();
    var line = 0;
    var col  = 0;
    if (! doc.setContent(input, false, err, line, col))
      throw "error reading XML at " + line + ":" + col + "\n" + err;

    var envelope = doc.documentElement();
    if (envelope.tagName() != "SOAP-ENV:Envelope")
      throw "error: expected SOAP-ENV:Envelope but found "
          + envelope.tagName();

    var body = envelope.firstChildElement();
    if (body.tagName() != "SOAP-ENV:Body")
      throw "error: expected SOAP-ENV:Body but found " + body.tagName();

    var child = body.childNodes();
    if (child.size() < 1)
      return null;
    else if (child.size() == 1)
      output = xml2js(child.at(0));
    else
    {
      output = new Array();
      for (var i = 0; i < child.size(); i++)
        output[i] = xml2js(child.at(i));
    }

    return output;
  }
  catch (e) {
     print("soap2js - exception at line " + e.lineNumber + ": " + e);
  }
}
