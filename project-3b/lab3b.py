#!/usr/bin/python2.7 
import sys
import csv


class FileSysAuditor:
    _data = []
    _inodes = {}
    _free_inodes = {}
    _bfrees = []
    _ifrees = []
    _dirents = {}
    _indirects = {}
    _superblock = []
    _block_counts = {}

    _num_blocks = 0
    _block_size = 0
    _indirect_strs = ["BLOCK", "INDIRECT", "DOUBLE INDIRECT", "TRIPLE INDIRECT"]
    _indirect_offsets = [0, 12, 268, 65804]

    def __init__(self, file_name):
        self._data = csv.reader(open(file_name, 'rb'))
        self._initialize()

    def _initialize(self):
        for row in self._data:
            if row[0] == 'SUPERBLOCK':
                self._num_blocks = int(row[1])
                self._block_size = int(row[3])
                self._superblock = row
            if row[0] == 'INODE':
                self._inodes[int(row[1])] = row
            if row[0] == 'DIRENT':
                self._dirents[int(row[3])] = row
            if row[0] == 'INDIRECT':
                self._indirects[int(row[4])] = row
                self._indirects[int(row[5])] = row
            if row[0] == 'BFREE':
                self._bfrees.append(int(row[1]))
            if row[0] == 'IFREE':
                self._ifrees.append(int(row[1]))

    def check_block_and_inode_consistency(self):
        allocated_blocks = {}

        for key, value in self._inodes.iteritems():
            for i in range(12, 24, 1):
                offset = i - 12
                if int(value[i]) > 7:
                    if int(value[i]) in allocated_blocks:
                        print "DUPLICATE %s %s IN INODE %s AT OFFSET %d" % (self._indirect_strs[allocated_blocks[(int(value[i]))][1]], value[i], allocated_blocks[int(value[i])][0], allocated_blocks[((int(value[i])))][2])
                        print "DUPLICATE BLOCK %s IN INODE %s AT OFFSET %d" % (value[i], value[1], offset)
                    else:
                        allocated_blocks[(int(value[i]))] = [value[1], 0, i - 12]
                if int(value[i]) < 0 or int(value[i]) > self._num_blocks:
                    print "INVALID BLOCK %s IN INODE %s AT OFFSET %d" % (value[i], value[1], offset)
                elif 0 < int(value[i]) <= 7:
                    print "RESERVED BLOCK %s IN INODE %s AT OFFSET %d" % (value[i], value[1], offset)

            for j in range(24, 27, 1):
                if self._num_blocks > int(value[j]) > 7:
                    if int(value[j]) in allocated_blocks:
                        print "DUPLICATE %s %s IN INODE %s AT OFFSET %d" % (self._indirect_strs[allocated_blocks[(int(value[j]))][1]], value[j], allocated_blocks[int(value[j])][0], allocated_blocks[((int(value[j])))][2])
                        print "DUPLICATE %s BLOCK %s IN INODE %s AT OFFSET %d" % (self._indirect_strs[j - 23], value[j], value[1], self._indirect_offsets[j-23])
                    else:
                        allocated_blocks[(int(value[j]))] = [value[1], j-23]
                if int(value[j]) < 0 or int(value[j]) > self._num_blocks:
                    print "INVALID %s BLOCK %s IN INODE %s AT OFFSET %d" % (self._indirect_strs[j-23], value[j], value[1], self._indirect_offsets[j-23])
                elif 0 < int(value[j]) <= 7:
                    print "RESERVED %s BLOCK %s IN INODE %s AT OFFSET %d" % (self._indirect_strs[j-23], value[j], value[1], self._indirect_offsets[j-23])

        for block in range(8, self._num_blocks, 1):
            if block in self._inodes:
                continue
            elif block in self._bfrees:
                continue
            elif block in self._ifrees:
                continue
            elif block in self._indirects:
                continue
            elif block in allocated_blocks:
                continue
            else:
                print "UNREFERENCED BLOCK %d" % block

        for block in self._bfrees:
            if block in allocated_blocks:
                print "ALLOCATED BLOCK %d ON FREELIST" % block

        for inode in self._ifrees:
            if inode in self._inodes:
                print "ALLOCATED INODE %d ON FREELIST" % inode

    def get_data(self):
        return self._data

    def get_inodes(self):
        return self._inodes

    def get_free_inodes(self):
        return self._free_inodes

    def get_block_counts(self):
        return self._block_counts


if __name__ == '__main__':
    if len(sys.argv) is not 2:
        print >> sys.stderr, "Error: lab3b should only have 1 argument"
        exit(1)

    sysAuditor = FileSysAuditor(sys.argv[1])

    sysAuditor.check_block_and_inode_consistency()