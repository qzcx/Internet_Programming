import argparse
import os
import requests
import threading
import sys


def chunkIt(size, num):
    print "size:",size, "num: ", num
    avg = size / float(num)
    last = 0.0
    print avg
    while last < size:
        print (int(last),int(last + avg))
        last += avg

def chunkify(size,n):
    return [ (i,n) for i in xrange(n) ]

def split(size, n):
    print "size:",size, "num: ", n
    k, m = size / n, size % n
    ranges = []
    for i in xrange(n):
        ranges.append((i * k + min(i, m),(i + 1) * k + min(i + 1, m)))
    print ranges
    return ranges


if __name__ == "__main__":
    #ranges = split(10,3) 
    #print ranges[0][1]
    content = []
    content.insert(0,"hi")
    content.insert(1,"there")
    print "".join(content)
    #ranges = split(int(sys.argv[0]),int(sys.argv[1])

#print range(0,5)

