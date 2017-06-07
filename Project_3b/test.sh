#!/bin/bash
#
# sanity check script for Project 3B
#	tarball name
#	tarball contents
#	student identification 
#	makefile targets
#	error free build
#	make clean
#	make dist
#	make (default again)
#	usage message for bogus args
#	error message for nonexistent iput file
#
#	identical output for supplied test cases
#
LAB="lab3b"
README="README"
MAKEFILE="Makefile"
BASE="http://web.cs.ucla.edu/classes/spring17/cs111/projects"
TESTS=22
EXPECTEDS=""

FILEBASE=`pwd`/../../P3B_analyze/GRADING

PGM=./lab3b
PGMS="$PGM"

TIMEOUT=1

let errors=0

if [ -z "$1" ]
then
	echo usage: $0 your-student-id
	exit 1
else
	student=$1
fi

# make sure the tarball has the right name
tarball="$LAB-$student.tar.gz"
if [ ! -s $tarball ]
then
	echo "ERROR: Unable to find submission tarball:" $tarball
	exit 1
fi

# make sure we can untar it
TEMP="/tmp/TestTemp.$$"
echo "... Using temporary testing directory" $TEMP
function cleanup {
	cd
	rm -rf $TEMP
	exit $1
}

mkdir $TEMP
cp $tarball $TEMP
cd $TEMP
echo "... untaring" $tarball
tar xvf $tarball
if [ $? -ne 0 ]
then
	echo "ERROR: Error untarring $tarball"
	cleanup 1
fi

# make sure we find all the expected files
echo "... checking for expected files"
for i in $README $MAKEFILE
do
	if [ ! -s $i ]
	then
		echo "ERROR: unable to find file" $i
		let errors+=1
	else
		echo "        $i ... OK"
	fi
done

# make sure the README contains name and e-mail
echo "... checking for submitter info in $README"
function idString {
	result=`grep $1 $README | cut -d: -f2 | tr -d \[:blank:\]`
	if [ -z "$result" ]
	then
		echo "ERROR - no $1 in $README";
		let errors+=1
	elif [ -z "$2" ]
	then
		# no match required
		echo "        $1 ... $result"
	else
		f1=`echo $result | cut -f1 -d,`
		f2=`echo $result | cut -f2 -d,`
		if [ "$f1" == "$2" ]
		then
			echo "$1 ... $f1"
		elif [ -n "$f2" -a "$2" == "$f2" ]
		then
			echo "$1 ... $f1,$f2"
		else
			echo "$1 does not include $2"
			let errors+=1
		fi
	fi
}

idString "NAME:"
idString "EMAIL:"
idString "ID:" $student

function makeTarget {
	result=`grep $1: $MAKEFILE`
	if [ $? -ne 0 ]
	then
		echo "ERROR: no $1 target in $MAKEFILE"
		let errors+=1
	else
		echo "        $1 ... OK"
	fi
}

echo "... checking for expected make targets"
makeTarget "clean"
makeTarget "dist"

# make sure we find files with all the expected suffixes
echo "... checking for other files of expected types"
for s in $EXPECTEDS
do
	names=`echo *.$s`
	if [ "$names" = '*'.$s ]
	then
		echo "ERROR: unable to find any .$s files"
		let errors+=1
	else
		for f in $names
		do
			echo "        $f ... OK"
		done
	fi
done

# make sure we can build the expected program
echo "... building default target(s)"
make 2> STDERR
RET=$?
if [ $RET -ne 0 ]
then
	echo "ERROR: default make fails RC=$RET"
	let errors+=1
fi
if [ -s STDERR ]
then
	echo "ERROR: make produced output to stderr"
	cat STDERR
	let errors+=1
fi

# check a make clean (successful, deletes products)
echo "... testing make clean"
make clean 2> STDERR
RET=$?
if [ $RET -ne 0 ]
then
	echo "ERROR: make clean fails RC=$RET"
	let errors+=1
fi
if [ -s STDERR ]
then
	echo "ERROR: make clean produced output to stderr"
	let errors+=1
fi

# check if a make clean eliminated all targets
for t in $tarball
do
	if [ -s $t ]
	then
		echo "ERROR: make clean leaves $t"
		let errors+=1
	fi
done
for t in *.pyc *.o *.dSYM
do
	if [ -s $t ]
	then
		echo "ERROR: make clean leaves $t"
		let errors+=1
	fi
done

# check if a make dist succeeds
echo "... testing make dist"
make dist 2> STDERR
RET=$?
if [ $RET -ne 0 ]
then
	echo "ERROR: make dist fails RC=$RET"
	let errors+=1
fi
if [ -s STDERR ]
then
	echo "ERROR: make dist produced output to stderr"
	let errors+=1
fi

# check if a make dist creates the tarball
if [ ! -s $tarball ]
then
	echo "ERROR: make dist does not produce $tarball"
	let errors+=1
fi

# check a make after a make clean
echo "... re-building default target(s)"
make 2> STDERR
RET=$?
if [ $RET -ne 0 ]
then
	echo "ERROR: default make fails RC=$RET"
	let errors+=1
fi
if [ -s STDERR ]
then
	echo "ERROR: make produced output to stderr"
	let errors+=1
fi

echo "... checking for expected products"
for p in $PGMS
do
	if [ ! -x $p ]
	then
		echo "ERROR: unable to find expected executable" $p
		let errors+=1
	else
		echo "        $p ... OK"
	fi
done

#echo ... checking for expected usage
#timeout $TIMEOUT $PGM > STDOUT 2> STDERR
#if [ $? -ne 1 ]; then
#	echo "ERROR: RC!=1 w/no args"
#	let errors+=1
#else
#	echo "        RC=1 ... OK"
#fi

echo ... checking for handling of nonexistent image
timeout $TIMEOUT $PGM NO_SUCH_FILE > STDOUT 2> STDERR
if [ $? -eq 0 ]; then
	echo "ERROR: no error w/invalid image"
	let errors+=1
elif [ ! -s STDERR ]; then
	echo "ERROR: no usage message"
	let errors+=1
else
	cat STDERR
	echo "        RC!=0, error message ... OK"
fi

# see if we can download the test cases
let b=0
while [ $b -le $TESTS ]
do
	pfx="P3B-test_"
	for s in csv err
	do
		if [ -s $FILEBASE/$pfx$b.$s ]; then
			cp $FILEBASE/$pfx$b.$s .
			ret=$?
		else
			wget $BASE/$pfx$b.$s > /dev/null 2> /dev/null
			ret=$?
		fi
		if [ $ret -ne 0 ]; then
			echo "Unable to download testcase $f"
		fi
	done

	echo "... executing test case $pfx$b"
	timeout $TIMEOUT $PGM $pfx$b.csv | sort > TEST.out
	sort $pfx$b.err > GOLDEN.out
	cmp GOLDEN.out TEST.out
	if [ $? -ne 0 ]; then
		echo "        incorrect output for $b.csv"
		echo "YOUR OUTPUT ..."
		cat TEST.out
		echo "EXPECTED OUTPUT ..."
		cat GOLDEN.out
		echo "=========="
		let errors+=1
	else
		echo "        all" `wc -l < GOLDEN.out` "lines agree"
	fi
	let b+=1
done

# that's all the tests I could think of
echo
if [ $errors -eq 0 ]; then
	echo "SUBMISSION $tarball ... passes sanity check"
	cleanup 0
else
	echo "SUBMISSION $tarball ... fails sanity check with $errors errors!"
	echo
	echo
	cleanup -1
fi