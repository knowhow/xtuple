/* This is an example script showing query execution and error checking.

  This script shows how to execute a simple database query, walk through
  the results, and report problems to the user. It is designed to be used
  as an initMenu script.

  It queries the database for currencies without exchange rates for
  the current day. As each row is returned, a message is displayed
  for the user.
*/
var fxqry = toolbox.executeQuery("SELECT curr_abbr, curr_rate "
                               + "  FROM curr_symbol LEFT OUTER JOIN"
                               + "       curr_rate ON (curr_symbol.curr_id=curr_rate.curr_id"
                               + "                 AND CURRENT_DATE BETWEEN curr_effective AND curr_expires + 1)"
                               + " WHERE curr_rate IS NULL;");
while (fxqry.next())
  QMessageBox.warning(mainwindow, qsTr("No exchange rate"),
                     qsTr("%1 has no exchange rate for today.")
                       .arg(fxqry.value("curr_abbr")));
if (fxqry.lastError().type != QSqlError.NoError)
  QMessageBox.critical(mainwindow, qsTr("Database Error"),
                       qsTr("There was a database error while checking for "
                          + "exchange rates during startup:%1")
                       .arg(fxqry.lastError().text));
