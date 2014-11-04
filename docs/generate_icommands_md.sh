#!/bin/bash -e

SUFFIX=md

DETECTEDDIR=$( cd $(dirname $0) ; pwd -P )

TARGETDIR=$DETECTEDDIR/iCommands
mkdir -p $TARGETDIR

TARGETFILE=$DETECTEDDIR/iCommands/user.$SUFFIX
echo > $TARGETFILE
echo "# iCommands - User" >> $TARGETFILE

ihelp -a | grep "iRODS Version " | awk '{print $6}' | while read x ; do

  if [ "$x" = "iadmin" ] ; then
    # iadmin has subcommands
    TARGETFILE=$DETECTEDDIR/iCommands/administrator.$SUFFIX
    echo > $TARGETFILE
    echo "# iCommands - Administrator" >> $TARGETFILE
    echo "# $x" >> $TARGETFILE
    echo "<pre>" >> $TARGETFILE
    ihelp $x | sed \$d >> $TARGETFILE
    echo "</pre>" >> $TARGETFILE
    echo >> $TARGETFILE
    ihelp $x | grep "^ " | grep -v "^     " | awk '{print $1}' | while read y ; do
      echo "## $y" >> $TARGETFILE
      echo "<pre>" >> $TARGETFILE
      $x h $y >> $TARGETFILE
      echo "</pre>" >> $TARGETFILE
      echo >> $TARGETFILE
    done
  else
    # all other icommands
    TARGETFILE=$DETECTEDDIR/iCommands/user.$SUFFIX
    echo "# $x" >> $TARGETFILE
    echo "<pre>" >> $TARGETFILE
    ihelp $x | sed \$d >> $TARGETFILE
    echo "</pre>" >> $TARGETFILE
    echo >> $TARGETFILE
  fi

done
