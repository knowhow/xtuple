var _error = mywindow.findChild("_error");
var _input = mywindow.findChild("_input");
var _output = mywindow.findChild("_output");
var _proc;
var _program = mywindow.findChild("_program");
var _restart = mywindow.findChild("_restart");
var _var = mywindow.findChild("_var");

function sProcError(theError)
{
  QMessageBox.warning(mywindow, "warning", "Got an error: " + theError);
  try {
    if (theError == QProcess.FailedToStart)
    {
      var args = _program.text.split(/\s/);
      var cmd = args.shift();
      var fi = new QFileInfo(cmd);

      if (fi.isRelative())
      {
        if (cmd.match(/^\./))
        {
          cmd = fi.fileName();
          fi = new QFileInfo(cmd);
        }

        var dir = fi.absoluteDir();
        var simplename = fi.fileName();
        var fullpath = fi.canonicalFilePath();
        while (! QFile.exists(fullpath) && ! dir.isRoot())
        {
          dir.cdUp();
          fullpath = dir.absoluteFilePath(simplename);
          print("looking at " + fullpath);
        }
        if (QFile.exists(fullpath))
        {
          _program.text = fullpath + " " + args.join(" ");
          _proc.start(cmd, args);
        }
        else
          QMessageBox.warning(mywindow, "Program Not Started", "Cannot start " + cmd);
      }
      else
        QMessageBox.warning(mywindow, "Program Not Started", "Cannot start " + cmd);

    }
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sProcError exception @ " + e.lineNumber + ": " + e.message);
  }
}

function sRestart()
{
  try {
    _input.clear();
    _output.clear();
    _proc = new QProcess(mywindow);
    var args = _program.text.split(/\s/);
    var cmd  = args.shift();
    if (_var.plainText.length)
    {
      var env = QProcessEnvironment.systemEnvironment();
      var add = _var.plainText.split(/\s+/);
      for (var i = 0; i < add.length; i++)
      {
        var one = add[i].split("=");
        env[one[0]] = one[1];
      }
      _proc.setProcessEnvironment(env);
    }
    _proc.start(cmd, args);

    _input.returnPressed.connect(sWriteToProc);
    _proc.readyReadStandardOutput.connect(sReadFromProc);
    _proc.readyReadStandardError.connect(sReadFromProc);
    _proc["error(QProcess::ProcessError)"].connect(sProcError);
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sRestart exception @ " + e.lineNumber + ": " + e.message);
  }
}

function sReadFromProc()
{
  try {
    _input.clear();
    var stdout  = String(_proc.readAllStandardOutput());
    var stderr  = String(_proc.readAllStandardError());
    _output.plainText += String(stdout);
    _error.plainText  += String(stderr);
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sRestart exception @ " + e.lineNumber + ": " + e.message);
  }
}

function sWriteToProc()
{
  try {
    _output.plainText = _input.text + '\n===========================================\n';
    _proc.write(QByteArray(_input.text));
    _proc.write(QByteArray('\n'));
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sWriteToProc exception @ " + e.lineNumber + ": " + e.message);
  }
}

_program.returnPressed.connect(sRestart);
_restart.clicked.connect(sRestart);