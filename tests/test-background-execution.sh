#!/bin/sh

runprogram(){
../bin/shell <<-'EOF'
sleep 1 &
sleep 4
exit
EOF
}
#Exit if there is an error
set -e
rm -f test1.log test2.log test3.log
runprogram 2>&1 | tee -a test3.log

c=$(grep -c "Child Exited Successfully" test3.log)
two=2
if [ "$c" = "$two" ]
then
  echo "All children exited successfully!"
  exit 0;
else
  exit 1;
fi

rm -f bkgnd.log
