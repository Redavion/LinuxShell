#!/bin/sh


runprogram(){
../bin/shell <<-'EOF'
testnonexistent.sh
exit
EOF
exit 0 
}

#Exit if there is an error

rm -f test1.log test2.log test3.log
runprogram 2>&1 | tee netest.log

# from this point onwards the test will fail if any command fails
cnt=$(grep -c "Exec Failed" netest.log)
zero=1
if [ "$cnt" -ne "$one" ]
 then
  echo "The test failed because we did not see the Exec Failed message"
  exit 1
 fi
echo "Test2 - checking error in exec passed"
exit 0

rm -f netest.log
