#!/usr/bin/python

import sys

def main():
	if len(sys.argv < 2) :
		sys.stderr.write("Error. Usage: ./lab3b [csv file]");
		sys.stderr.flush();
		exit(1);
	else :
		filename = sys.argv[1];

	csvFile = open(filename, "r");

	