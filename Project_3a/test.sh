#!/bin/bash
#
# sanity check script for Project 3A
#	tarball name
#	tarball contents
#	student identification 
#	makefile targets
#	error free build
#	usage message for bogus args
#	error message for nonexistent iput file
#
#	identical output for trivial test case
#
LAB="lab3a"
README="README"
MAKEFILE="Makefile"
BASE="http://www.cs.pomona.edu/classes/cs134/projects"
IMAGE="trivial.img"
OUTPUT="trivial.csv"

EXPECTEDS=""

PGM=./lab3a
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
	elif [ -n "$2" -a "$2" != "$result" ]
	then
		echo "        $1 ... $result != $2"
	else
		echo "        $1 ... $result"
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

echo ... checking for expected usage
timeout $TIMEOUT $PGM > STDOUT 2> STDERR
if [ $? -ne 1 ]; then
	echo "ERROR: RC!=1 w/no args"
	let errors+=1
else
	echo "        RC=1 ... OK"
fi

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

# see if we can download the test case
echo ... Downloading test image $BASE/$IMAGE
wget $BASE/$IMAGE 2> /dev/null
xi=$?
echo ... Downloading test results $BASE/$OUTPUT
wget $BASE/$OUTPUT 2> /dev/null
xo=$?
if [ $xi -eq 0 -a $xo -eq 0 ]; then
	echo ... running $PGM on test image $IMAGE
	timeout $TIMEOUT $PGM $IMAGE | sort > TEST.csv
	for t in SUPER GROUP BFREE IFREE INODE DIRENT INDIRECT
	do
		grep $t $OUTPUT | sort > GOLDEN.csv
		grep $t TEST.csv > TEST
		cmp GOLDEN.csv TEST
		if [ $? -ne 0 ]; then
			echo "... $t: OUTPUT DOES NOT MATCH"
			let errors+=1
		else
			echo "... $t: all" `wc -l < GOLDEN.csv` "output lines match"
		fi
	done
else
	echo "ERROR: unable to download test cases from $BASE"
	let errors+=1
fi

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