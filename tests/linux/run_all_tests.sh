#!/bin/bash

echo Starting test procedure...

cd bin
echo The current directory is:
pwd

echo Cleanning garbage files...
find -name "*.tif" -exec rm -f {} \;
find -name "*.ilk" -exec rm -f {} \;
find -name "*.pdb" -exec rm -f {} \;
find -name "*.exe" -exec rm -f {} \;
echo Cleanning done.

AT_LEAST_ONE_TEST_FAILED=0
LOGFILE=/tmp/terralibtestlog.log

for i in $( ls * ); do
  if [ -x $i ]; then
    echo Now running $i test ...
    
    rm -f $LOGFILE
    
    TEST_FAILED=0
    ./$i &> $LOGFILE
    
    if [ "$?" -ne "0" ]; then 
      AT_LEAST_ONE_TEST_FAILED=1

      echo Test $i failed.
      
      echo ----------------------------------------------------------------
      cat $LOGFILE
      echo ----------------------------------------------------------------    
    else
      echo Test $i OK.
    fi
  fi
done

if [ $AT_LEAST_ONE_TEST_FAILED -ne "0" ]; then
  echo ----------------------------------------------------------------
  echo Test procedure finished with errors.
  exit 1
else
  echo Test procedure finished with no errors.
  exit 0
fi
