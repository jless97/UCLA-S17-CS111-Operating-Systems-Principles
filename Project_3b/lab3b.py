#!/usr/local/cs/bin/python3

import sys

class superBlock:
	totalNumBlocks__ = 0
	totalNumInodes__ = 0
	blockSize__ = 0
	inodeSize__ = 0
	blocksPerGroup__ = 0
	inodesPerGroup__ = 0

	def __init__(self, totalNumBlocks=0, totalNumInodes=0, blockSize=0, inodeSize=0, blocksPerGroup=0, inodesPerGroup=0):
		self.totalNumBlocks__ = int(totalNumBlocks)
		self.totalNumInodes__ = int(totalNumInodes)
		self.blockSize__ = int(blockSize)
		self.inodeSize__ = int(inodeSize)
		self.blocksPerGroup__ = int(blocksPerGroup)
		self.inodesPerGroup__ = int(inodesPerGroup)

class inode:
	inodeNum__ = 0
	fileType__ = 'f'
	mode__ = 0
	owner__ = 0
	group__ = 0
	linkCount__ = 0

	def __init__(self, inodeNum, fileType, mode, owner, group, linkCount):
		self.inodeNum__ = int(inodeNum)
		self.fileType__ = fileType
		self.mode__ = mode
		self.owner__ = owner
		self.group__ = int(group)
		self.linkCount__ = int(linkCount)

class dataBlock:
	blockNum__ = 0
	blockType__ = ""
	inodeNum__ = 0
	offset__ = 0

	def __init__(self, blockNum, blockType, inodeNum, offset):
		self.blockNum__ = int(blockNum)
		self.blockType__ = blockType
		self.inodeNum__ = int(inodeNum)
		self.offset__ = int(offset)

class directory:
	parentInode__ = 0
	refInode__ = 0
	dirName__ = ""

	def __init__(self, parentInode, refInode, dirName):
		self.parentInode__ = int(parentInode)
		self.refInode__ = int(refInode)
		self.dirName__ = dirName

# Globals
freeBlocks = []
freeInodes = []
dataBlocks = []
indirBlocks = []
inodes = []
directories = []
exit_status = 0

def isFreeBlock(bn) :
	for block in freeBlocks:
		if (block == bn):
			return True
	return False

def isDataBlock(bn,sb) :
	if (bn > sb.totalNumBlocks__):	# block number out of range
		return 2 
	elif (bn < 8):	# block number indicates a reserved block
		return 3
	else:
		if (isFreeBlock(bn)): # block number indicates a free block
			return 4
	return 1 # is a data block

def isAllocated(bn) :
	for allocated in dataBlocks:
		if (bn == allocated.blockNum__):
			return True
	return False

def checkBlocks(sb):
    	global exit_status
	cur = 0
	for block in dataBlocks:
		if (isDataBlock(block.blockNum__, sb) == 2):
            		exit_status = 2
			print("INVALID {} {} IN INODE {} AT OFFSET {}".format(block.blockType__, block.blockNum__, block.inodeNum__, block.offset__))
		elif (isDataBlock(block.blockNum__, sb) == 3):
            		exit_status = 2
			print("RESERVED {} {} IN INODE {} AT OFFSET {}".format(block.blockType__, block.blockNum__, block.inodeNum__, block.offset__))
		elif(isFreeBlock(block.blockNum__)):
            		exit_status = 2
			print("ALLOCATED BLOCK {} ON FREELIST".format(block.blockNum__))
		else:
			next = 0
			for otherBlock in dataBlocks:
				if ((block.blockNum__ == otherBlock.blockNum__) and cur != next):
                    			exit_status = 2
					print("DUPLICATE {} {} IN INODE {} AT OFFSET {}".format(otherBlock.blockType__, otherBlock.blockNum__, otherBlock.inodeNum__, otherBlock.offset__))
				next += 1
		cur += 1

	end = sb.totalNumBlocks__
	for allBlocks in range (0, end):
		if ((isFreeBlock(allBlocks) == False) and (isDataBlock(allBlocks, sb) == 1) and (isAllocated(allBlocks) == False)):
            		exit_status = 2
			print("UNREFERENCED BLOCK {}".format(allBlocks))

def isFreeInode(sb) :
    	global exit_status
	allocated = []
	unallocated = []

	for inode in inodes:
		if inode.fileType__ == 'f' or inode.fileType__ == 'd':
			allocated.append(inode.inodeNum__)

	for inode in allocated:
		if inode in freeInodes:
            		exit_status = 2
			print("ALLOCATED INODE", inode, "ON FREELIST")

	for inode in freeInodes:
		if inode not in allocated:
			unallocated.append(inode)

	for inode in range(1, sb.totalNumInodes__):
		if inode not in unallocated and inode not in allocated:
			if inode > 10:
                		exit_status = 2
				print("UNALLOCATED INODE", inode, "NOT ON FREELIST")

def checkDirectory(sb) :
    	global exit_status
	parentMap = [[None]]*sb.totalNumInodes__
	childMap = [None]*sb.totalNumInodes__
	# check link counts
	for inode in inodes:
		links = inode.linkCount__
		count = 0
		for dirEnt in directories:
			if (dirEnt.refInode__ == inode.inodeNum__):
				count += 1
		if count != links:
           		exit_status = 2
			print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(inode.inodeNum__, count, inode.linkCount__))

	# check if referenced inodes are valid/allocated/correct
	for directory in directories:
		# '.' should link to the inode itself
		if ((directory.dirName__ == "'.'") and (directory.refInode__ != directory.parentInode__)):
            		exit_status = 2
			print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(directory.parentInode__, directory.refInode__, directory.parentInode__))
		# '..' should link to the parent
		if (directory.dirName__ == "'..'"):
			foundParent = 0
			foundMatch = 0
			for de in directories:
				if (de.refInode__ == directory.parentInode__):
					if (de.parentInode__ != directory.refInode__):
                        			exit_status = 2
						print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(directory.parentInode__, directory.refInode__, de.parentInode__))
						break
				if (de.parentInode__ == directory.refInode__):
					foundParent = 1
					if (de.refInode__ == directory.parentInode__):
						foundMatch = 1
						break
			if ((foundParent == 1) and (foundMatch == 0)):
                		exit_status = 2
				print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(directory.parentInode__, directory.refInode__, directory.parentInode__))

		allocated = 0
		if ((directory.refInode__ > sb.totalNumInodes__) or (directory.refInode__ < 0)) :
            		exit_status = 2
			print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(directory.parentInode__, directory.dirName__, directory.refInode__))
			break
		for inode in inodes:
			if (inode.inodeNum__ == directory.refInode__):
				allocated = 1
				break
		if (allocated == 0):
            		exit_status = 2
			print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(directory.parentInode__, directory.dirName__, directory.refInode__))

def main():
	if (len(sys.argv) != 2) :
		print("Error. Usage: ./lab3b [csv file]", file=sys.stderr)
    		sys.exit(1)
	else :
		filename = sys.argv[1]

	try: 
		csvFile = open(filename, "r")
	except:
		print("Error: failed opening file {}".format(filename), file=sys.stderr)
		sys.exit(1)

	list = csvFile.readlines();

	for line in list:
		if (line[0:10] == "SUPERBLOCK"):
			line = line.strip()
			sbInfo = line.split(',') # split the superblock info
			sb = superBlock(sbInfo[1], sbInfo[2], sbInfo[3], sbInfo[4], sbInfo[5], sbInfo[6]);
		elif (line[0:5] == "BFREE"):
			line = line.strip()
			fbn = line.split(',')	# free block number
			freeBlocks.append(int(fbn[1]))
		elif (line[0:5] == "IFREE"):
			line = line.strip()
			fin = line.split(',') # free inode number
			freeInodes.append(int(fin[1]))
		# store dataBlock block number, and types of blocks
		elif (line[0:5] == "INODE"):
			line = line.strip()
			inodeInfo = line.split(',')
			# Every INODE line add to the inode list
			inode_elem = inode(inodeInfo[1], inodeInfo[2], inodeInfo[3], inodeInfo[4], inodeInfo[5], inodeInfo[6]);
			inodes.append(inode_elem)

			cur = 11
			for bn in inodeInfo[cur:24]:
				if (bn != '0'):
					print(bn)
					db = dataBlock(bn, "BLOCK", inodeInfo[1], cur-12)
					dataBlocks.append(db)
				cur += 1
			if (inodeInfo[24] != '0'):
				db = dataBlock(inodeInfo[24], "INDIRECT BLOCK", inodeInfo[1], 12)
				dataBlocks.append(db)
			if (inodeInfo[25] != '0'):
				db = dataBlock(inodeInfo[25], "DOUBLE INDIRECT BLOCK", inodeInfo[1], 12+256)
				dataBlocks.append(db)
			if (inodeInfo[26] != '0'):
				db = dataBlock(inodeInfo[26], "TRIPPLE INDIRECT BLOCK", inodeInfo[1], 12+256+256*256)
				dataBlocks.append(db)
		elif (line[0:8] == "INDIRECT"):
			line = line.strip()
			indirInfo = line.split(',')
			if (indirInfo[2] == 1):
				db = dataBlock(indirInfo[5], "BLOCK", indirInfo[1], indirInfo[3])
				dataBlocks.append(db)
			elif (indirInfo[2] == 2):
				db = dataBlock(indirInfo[5], "SINGLE INDIRECT BLOCK", indirInfo[1], indirInfo[3])
				dataBlocks.append(db)
			else:
				db = dataBlock(indirInfo[5], "DOUBLE INDIRECT BLOCK", indirInfo[1], indirInfo[3])
				dataBlocks.append(db)
		elif (line[0:6] == "DIRENT"):
			line = line.strip()
			dirInfo = line.split(',')
			dirEnt = directory(dirInfo[1], dirInfo[3], dirInfo[6])
			directories.append(dirEnt)

	#for block in dataBlocks:
	#	print("block number = {} inode number = {}".format(block.blockNum__, block.inodeNum__))

	checkBlocks(sb)

	isFreeInode(sb)

	checkDirectory(sb)
	'''
	for db in dataBlocks:
		print(db.blockNum__)
	for b in freeBlocks:
		print(b)'''
	#print (sb.totalNumBlocks__)

   	# Exit status: 0 => success, 1 => errors, 2 => success, but inconsistencies
    	sys.exit(exit_status)

if __name__ == "__main__": main()
