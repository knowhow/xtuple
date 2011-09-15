#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#
PROG=`basename $0`
VERFILE=guiclient/version.cpp
BUILD=false
DEMO=false
BASEDIR=`pwd`
if [ "$BASEDIR" = . ] ; then
  BASEDIR=`pwd`
fi

usage() {
  echo "$PROG -h"
  echo "$PROG -b -d [ -x ]"
  echo
  fmt <<EOF
$PROG bundles the xTuple applications for release on the current platform
(currently only Mac OS X is supported).
$PROG reads $VERFILE to get release information used to
name the release package.
EOF
  echo
  echo "-h	get this usage message"
  echo "-b	force build (run qmake and make before bundling)"
  echo "	$PROG runs qmake and make if the required binaries are missing "
  echo "	from the bin directory even if -b is not specified."
  echo "-d <file>	build and bundle a demo release, including the help file named <file>"
  echo "	The help file for the demo is /not/ the regular application help"
  echo "	To build a demo you must first edit some of the source files"
  echo "-x	turn on shell debugging output"
}

bundle() {
  if [ "$#" -ne 2 ] ; then
    echo "Illegal call to bundle: bad arg count"
    echo "usage: bundle app dir_to_put_it_in"
    return 1
  fi

  STARTDIR=`pwd`

  APPNAME="$1"
  if expr "$APPNAME" : ".*\.app" ; then
    BINARY="$APPNAME"
    APPNAME=`basename "$APPNAME" .app`
  else
    BINARY="${APPNAME}.app"
  fi

  if $DEMO ; then
    BUNDLENAME="${APPNAME}-${VERSION}Demo-MACUniversal"
  else
    BUNDLENAME="${APPNAME}-${VERSION}-MACUniversal"
  fi
  BUNDLEDIR="${2}/${BUNDLENAME}"

  if [ -d "$BUNDLEDIR" ] ; then
    rm -rf "$BUNDLEDIR"                                               || return 3
  fi

  mkdir "$BUNDLEDIR"                                                  || return 3

  if ! $DEMO ; then
    cp -R -L "$BASEDIR/bin/$BINARY" "$BUNDLEDIR"                      || return 4
    mkdir "$BUNDLEDIR/${BINARY}/Contents/Resources/helpXTupleGUIClient" || return 5
    cp "$BASEDIR/share/empty_help.html" \
       "$BASEDIR/share/XTupleGUIClient.adp" \
       "$BUNDLEDIR"/"${BINARY}"/Contents/Resources/helpXTupleGUIClient || return 5
  else
    mv "$BASEDIR/bin/$BINARY" "$BUNDLEDIR"/"${APPNAME}_Demo.app"      || return 4
    BINARY="${APPNAME}_Demo.app"
    if [ -d "$HELPFILE" ] ; then
      cp -R -L "$HELPFILE" \
      "$BUNDLEDIR"/bin/"${BINARY}"/Contents/Resources/helpXTupleGUIClient || return 5
    elif [ -f "$HELPFILE" ] ; then
      CURRDIR=`pwd`
      cd "$BUNDLEDIR"/"${BINARY}"/Contents/Resources                  || return 5
      jar xf "$HELPFILE"                                              || return 5
      if [ ! -d helpXTupleGUIClient ] ; then
	echo "$PROG: help file $HELPFILE was not installed properly in demo client"
	return 5
      fi
      cd "$CURRDIR"                                                   || return 5
    fi
  fi

  cd "$BUNDLEDIR"/..                                                  || return 5
  if [ -f "$BUNDLENAME".dmg ] && ! rm "$BUNDLENAME".dmg ; then
    return 5
  fi
  if ! hdiutil create -fs HFS+ -volname "$BUNDLENAME" -srcfolder "$BUNDLENAME" "$BUNDLENAME".dmg ; then
    return 5
  fi

  cd $STARTDIR                                                        || return 5
}

ARGS=`getopt hbd:x $*`
if [ $? != 0 ] ; then
  usage
  exit 1
fi
set -- $ARGS

while [ "$1" != -- ] ; do
  case "$1" in
    -h)
	usage
	exit 0
	;;
    -b)
	BUILD=true
	;;
    -d)
	DEMO=true
	HELPFILE=$2
	if ! expr ${HELPFILE} : "\/" ; then
	  HELPFILE=`pwd`/"$HELPFILE"
	fi
	if egrep -i "_evaluation.*FALSE" guiclient/main.cpp ; then
	  echo The sources do not appear to have been modified to build a demo
	  echo
	  echo Change guiclient/main.cpp: set _evaluation to TRUE
	  echo Change common/login2.cpp: set the port in _evalDatabaseURL
	  echo Change common/login2.cpp: set the passwd to the proper value
	  exit 1
	fi
	shift
	;;
    -x)
	set -x
	;;
    *)
	usage
	exit 1
	;;
  esac
  shift
done
shift # past the --

if [ "$#" -gt 0 ] ; then
  echo "$PROG: ignoring extra arguments $*"
fi

if [ `uname -s` != Darwin ] ; then
  echo "$PROG: only supports Macintosh OS X (Darwin) at this time"
  usage
  exit 2
fi

if [ -z "$QTDIR" ] ; then
  for TRYME in `echo $PATH | tr : ' '` ; do
    TRYME=`dirname $TRYME`
    if [ -e "$TRYME/bin/qmake" -a -e "$TRYME/lib/libQtCore.dylib" ] ; then
      export QTDIR="$TRYME"
      break;
    fi
  done
  if [ -n "QTDIR" ] ; then
    echo "$PROG: guessing that Qt is installed in $QTDIR"
  else
    echo "$PROG: Cannot run properly without the QTDIR environment variable set"
    exit 2
  fi
fi
if [ -z "$PGDIR" ] ; then
  for TRYME in `echo $PATH | tr : ' '` ; do
    TRYME=`dirname $TRYME`
    if [ -e "$TRYME/bin/psql" -a -e "$TRYME/lib/libpq.a" ] ; then
      export PGDIR="$TRYME"
      break;
    fi
  done
  if [ -n "PGDIR" ] ; then
    echo "$PROG: guessing that Qt is installed in $PGDIR"
  else
    echo "$PROG: Cannot run properly without the PGDIR environment variable set"
    exit 2
  fi
fi

if [ ! -e "$BASEDIR"/bin/xtuple.app ] ; then
  if ! $BUILD ; then
    echo "Building even though not explicitly told to do so"
  fi
  BUILD=true
fi

if [ ! -f "$VERFILE" ] ; then
  echo "Could not find $VERFILE"
  exit 2
fi
VERSION=`awk '/ _Version / { split($0,vers,"\""); print vers[2]}' $VERFILE | \
         tr -d [:blank:]`

if $BUILD ; then
  cd "$BASEDIR"                                                         || exit 3
  qmake                                                                 || exit 3
  make                                                                  || exit 3
fi

cd "$BASEDIR"/guiclient                                                 || exit 4
if [ `set -o | awk '/xtrace/ {print $2}'` = "on" ] ; then
  sh -x ./fixPackage                                                    || exit 4
else
  ./fixPackage                                                          || exit 4
fi
cd "$BASEDIR"/bin                                                       || exit 4
bundle xtuple "${BASEDIR}/.."                                           || exit 4

echo "DONE!"
