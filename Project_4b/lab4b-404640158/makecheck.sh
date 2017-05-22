#!/bin/bash
# Smoke-tests for make check

# Invalid Arguments Test
./lab4b --bogus &>/dev/null
if [[ $? -eq 1 ]] ; then
echo "Success: ./lab4b returns correct value when given invalid arguments" >> success_log.txt
else
echo "Error: ./lab4b returns incorrect value when given invalid arguments" >> failure_log.txt
fi

# Create log file test and processes commands in log
./lab4b --period=5 --log="log.txt" <<-EOF &>/dev/null
SCALE=F
PERIOD=1
START
STOP
OFF
EOF

if [ ! -s log.txt ] ; then
echo "Error: ./lab4b doesn't create log file" >> failure_log.txt
else
echo "Success: ./lab4b creates log file" >> success_log.txt
fi

# If success, then print success messages, otherwise error messages

if [[ -s failure_log.txt ]] ; then
echo "Failure: One or more test cases failed"
cat failure_log.txt
else
echo "Success: All test cases succeeded"
cat success_log.txt
fi
rm -f failure_log.txt success_log.txt