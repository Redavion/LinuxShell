#!/bin/sh

runprogram(){
../bin/shell <<-'EOF'
python testapp1.py || python testapp2.py | tee tmp.log
exit
EOF

t1=$(cat test1.log)
t2=$(cat test2.log)
a1="test1"
a2="test2"
echo "Expected Output for (testapp1.py, testapp2.py): ("$a1", "$a2")"
echo "Tests Output (testapp1.py, testapp2.py): ("$t1", "$t2")"

if [ "$t1" = "$a1" ] && [ "$t2" = "$a2" ]
then
  echo "File outputs are equal!"
  exit 0;
else
  echo "File outputs are different!" 
  exit 1;
fi
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

rm -f test3.log
