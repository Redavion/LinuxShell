#!/bin/sh

#Exit if there is an error
set -e
../bin/shell <<-'EOF'
cd ..
cd tests
python testapp.py
exit
EOF

a=$(cat test.log)
b="test"
echo "Expected Output:" $b
  echo "Program Output:" $a
  if [ "$a" = "$b" ]
    then
      echo "Success!"
      $(rm test.log)
    else
      echo "Fail" 
      exit 1;
  fi
exit 0 
