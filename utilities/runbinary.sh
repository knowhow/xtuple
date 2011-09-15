#!/bin/sh
#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

# Get the path to this program. All the libraries and
# executables we need should be in the same location.
XTUPLE_DIR=`dirname $0`
XTUPLE_DIR=`(cd $XTUPLE_DIR; /bin/pwd)`

# Get the name of the executable we are supposed
# to be running.
XTUPLE_EXE=`basename $0 .sh`

# Check to see if LD_LIBRARY_PATH is already set with
# a value. If so then we will prepend our values.
if [ -z $LD_LIBRARY_PATH ]; then
  LD_LIBRARY_PATH="$XTUPLE_DIR"
else
  LD_LIBRARY_PATH="$XTUPLE_DIR":$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH

# Execute the real binary we want now that our
# environment has been setup correctly.
"$XTUPLE_DIR/$XTUPLE_EXE.bin" $@
