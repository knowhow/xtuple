#!/bin/sh

UI_DIR=ui

if [ -z $1 ]; then
  echo "$0: No file to convert specified."
  exit
fi

base=`basename $1 .ui`

uifile=$base.ui

if  [ ! -f $uifile ]; then
  echo "$0: Could not find UI file $uifile."
  exit;
fi

cppfile=$UI_DIR/$base.cpp
hfile=$UI_DIR/$base.h

if [ ! -f $cppfile ]; then
  echo "$0: Could not find CPP file $cppfile."
  exit
fi
if [ ! -f $hfile ]; then
  echo "$0: Could not find H file $hfile."
  exit
fi

uihfile=$base.ui.h

if  [ ! -f $uihfile ]; then
  uihfile=
fi


mv $uifile $uifile.3
uic3 -convert $uifile.3 > $uifile
mv $cppfile $base.cpp
mv $hfile $base.h
if [ $uihfile ]; then
  mv $uihfile $uihfile.3
  cat $uihfile.3 >> $base.cpp
  cvs rm $uihfile
fi
cvs add $base.cpp $base.h

