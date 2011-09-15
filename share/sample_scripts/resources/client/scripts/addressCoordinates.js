mywindow.findChild("_close").clicked.connect(mywindow, "close");

var _addr = mywindow.findChild("_addr");
var googleapikey = "test";

function getCoords(input)
{
  var url = "http://maps.google.com/maps/geo?"
          + "q=" + _addr.line1() // q (required) — The address that you want to geocode.
          + ",+" + _addr.city()
          + ",+" + _addr.state()
          + "&key=" + googleapikey  // key (required) — Your API key.
          + "&sensor=false"         // sensor (required) — Indicates whether or not the geocoding request comes from a device with a location sensor. This value must be either true or false.
          + "&output=csv"           // output (required) — The format in which the output should be generated. The options are xml, kml, csv, or (default) json.
                                    // gl (optional) — The country code ... as a ... two-character value.
          ;

  url = url.replace(/( )+/g, "+");

  var response = "200,6,42.730,-73.690"; // http status, accuracy, lat, long

  var coords = response.split(",");

  mywindow.findChild("_latitude").text  = Math.abs(coords[2]);
  mywindow.findChild("_longitude").text = Math.abs(coords[3]);
  mywindow.findChild("_latDir").text  = (coords[2] < 0) ? "S" : "N";
  mywindow.findChild("_longDir").text = (coords[3] < 0) ? "W" : "E";
}

_addr.newId.connect(getCoords);