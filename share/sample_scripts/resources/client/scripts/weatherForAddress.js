/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

debugger;

/* soap2js provides a facility to convert arbitrary SOAP messages and other
   XML files to a javascript object. elements in the XML become properties
   of the javascript object, including the full nested structure.
 */
include("soap2js");

var _addr       = mywindow.findChild("_addr");
var _conditions = mywindow.findChild("_conditions");
var _datetime   = mywindow.findChild("_datetime");
var _err        = mywindow.findChild("_err");
var _netmgr     = new QNetworkAccessManager(mywindow);
var _output     = mywindow.findChild("_output");
var _temp       = mywindow.findChild("_temp");

function sSendQuery()
{
  try
  {
    _output.setPlainText("Searching...");
    var query = new Object;
    query.p = _addr.postalCode();
    query.u = 'f';

    var url = new QUrl('http://weather.yahooapis.com/forecastrss');
    url.setQueryItems(query);

    var netrequest = new QNetworkRequest();
    netrequest.setUrl(url);

    _netmgr.get(netrequest);
  }
  catch (e) {
    print("sSendQuery - exception at line " + e.lineNumber + ": " + e);
  }
}

function sGetResponse(netreply)
{
  try
  {
    if (netreply.error())
    {
      QMessageBox.warning(mywindow, qsTr("Network Error"),
                          qsTr("<p>The request for weather information "
                             + "returned error %1<br>%2")
                             .arg(netreply.error())
                             .arg(netreply.errorString()));
      return;
    }

    var xmlstring = netreply.readAll();
    _output.setPlainText(xmlstring);

    var weather = xml2js(xmlstring);
    var condition = weather.rss.channel.item.yweather_condition;
    if (condition == null)
    {
      var na = qsTr("N/A");
      _conditions.text = na;
      _temp.text       = na;
      _datetime.text   = na;
      _err.text        = weather.rss.channel.item.title.xmltext + " " +
                         weather.rss.channel.item.description.xmltext;
    }
    else
    {
      _conditions.text = condition.xmlattribute.text;
      _temp.text       = condition.xmlattribute.temp;
      _datetime.text   = condition.xmlattribute.date;
      _err.text        = "";
    }
  }
  catch (e)
  {
    print("sGetResponse - exception at line " + e.lineNumber + ": "
        + e);
  }
}

_addr["newId(int)"].connect(sSendQuery);
_netmgr.finished.connect(sGetResponse);
