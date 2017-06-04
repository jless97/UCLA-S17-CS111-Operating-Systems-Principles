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
		self.totalNumInodes__ = totalNumInodes
		self.blockSize__ = blockSize
		self.inodeSize__ = inodeSize
		self.blocksPerGroup__ = blocksPerGroup
		self.inodesPerGroup__ = inodesPerGroup

class inode:
	inodeNum__ = 0
	fileType__ = 'f'
	mode__ = 0
	owner__ = 0
	group__ = 0
	linkCount__ = 0

	def __init__(self, inodeNum, fileType, mode, owner, group, linkCount):
		self.inodeNum__ = inodeNum
		self.fileType__ = fileType
		self.mode__ = mode
		self.owner__ = owner
		self.group__ = group
		self.linkCount__ = linkCount

class dataBlock:
	blockNum__ = 0
	blockType__ = ""
	inodeNum__ = 0
	offset__ = 0

	def __init__(self, blockNum, blockType, inodeNum, offset):
		self.blockNum__ = int(blockNum)
		self.blockType__ = blockType
		self.inodeNum__ = inodeNum
		self.offset__ = offset

freeBlocks = []
freeInodes = []
dataBlocks = []
indirBlocks = []

def isFreeBlock(bn) :
	for blocks in freeBlocks:
		if (blocks == bn):
			return True
	return False

def isDataBlock(bn,sb) :
	#print(sb.totalNumBlocks__)
	if (bn > sb.totalNumBlocks__):	# block number out of range
		return 2 
	elif (bn < 8):	# block number indicates a reserved block
		return 3
	else:
		if (isFreeBlock(bn)): # block number indicates a free block
			return 4
	return 1 # is a data block


def checkBlocks(sb):
	cur = 0
	for block in dataBlocks:
		if (isDataBlock(block.blockNum__, sb) == 2):
			print("INVALID {} {} IN INODE {} AT OFFSET {}".format(block.blockType__, block.blockNum__, block.inodeNum__, block.offset__))
		elif (isDataBlock(block.blockNum__, sb) == 3):
			print("RESERVED {} {} IN INODE {} AT OFFSET {}".format(block.blockType__, block.blockNum__, block.inodeNum__, block.offset__))
		elif ((isDataBlock(block.blockNum__, sb) != 1) and (isFreeBlock(bn) == False)):
			print("UNREFERENCED BLOCK {}".format(block.blockNum__))
		elif(isFreeBlock(block.blockNum__)):
			print("ALLOCATED BLOCK {} ON FREELIST".format(block.blockNum__))
		else:
			next = 0
			for otherBlock in dataBlocks:
				if ((block.blockNum__ == otherBlock.blockNum__) and cur != next):
					print("DUPLICATE {} {} IN INODE {} AT OFFSET {}".format(otherBlock.blockType__, otherBlock.blockNum__, otherBlock.inodeNum__, otherBlock.offset__))
				next += 1
		cur += 1

def main():
	if (len(sys.argv) != 2) :
		print("Error. Usage: ./lab3b [csv file]", file=sys.stderr)
		exit(1)
	else :
		filename = sys.argv[1]

	try: 
		csvFile = open(filename, "r")
	except:
		print("Error: failed opening file {}".format(filename), file=sys.stderr)
		exit(1)

	list = csvFile.readlines();

	for line in list:
		if (line[0:10] == "SUPERBLOCK"):
			line = line.strip()
			sbInfo = line.split(',') # split the superblock info
			sb = superBlock(sbInfo[1], sbInfo[2], sbInfo[3], sbInfo[4], sbInfo[5], sbInfo[6]);
		elif (line[0:5] == "BFREE"):
			line = line.strip()
			fbn = line.split(',')	# free block number
			freeBlocks.append(fbn[1])
		elif (line[0:5] == "IFREE"):
			line = line.strip()
			fin = line.split(',') # free inode number
			freeInodes.append(fin[1])
		# store dataBlock block number, and types of blocks
		elif (line[0:5] == "INODE"):
			line = line.strip()
			inodeInfo = line.split(',')
			cur = 12
			for bn in inodeInfo[cur:24]:
				if (bn != '0'):
					#print(bn)
					db = dataBlock(bn, "BLOCK", inodeInfo[1], cur-12)
					dataBlocks.append(db)
				cur += 1
			if (inodeInfo[24] != '0'):
				#print(inodeInfo[24])
				db = dataBlock(inodeInfo[24], "INDIRECT BLOCK", inodeInfo[1], 12)
				dataBlocks.append(db)
			if (inodeInfo[25] != '0'):
				#print(inodeInfo[25])
				db = dataBlock(inodeInfo[25], "DOUBLE INDIRECT BLOCK", inodeInfo[1], 12+256)
				dataBlocks.append(db)
			if (inodeInfo[26] != '0'):
				#print(inodeInfo[26])
				db = dataBlock(inodeInfo[26], "TRIPPLE INDIRECT BLOCK", inodeInfo[1], 12+256+256*256)
				dataBlocks.append(db)
		elif (line[0:8] == "INDIRECT"):
			line = line.strip()
			indirInfo = line.split(',')
			#print(indirInfo[5])
			if (indirInfo[2] == 1):
				print(indirInfo[5])
				db = dataBlock(indirInfo[5], "BLOCK", indirInfo[1], indirInfo[3])
				dataBlocks.append(db)
			elif (indirInfo[2] == 2):
				db = dataBlock(indirInfo[5], "SINGLE INDIRECT BLOCK", indirInfo[1], indirInfo[3])
				dataBlocks.append(db)
			else:
				db = dataBlock(indirInfo[5], "DOUBLE INDIRECT BLOCK", indirInfo[1], indirInfo[3])
				dataBlocks.append(db)

	#for db in dataBlocks:
	#	print(db.blockNum__)
	#print (sb.totalNumBlocks__)
	checkBlocks(sb)


if __name__ == "__main__": main()
