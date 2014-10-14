import argparse
import os
import requests
import threading
import time

currentThread = 0
content = []
''' Downloader for a set of files '''
class Downloader:
    def __init__(self):
        ''' initialize the file where the list of URLs is listed, and the
        directory where the downloads will be stored'''
        self.args = None
        self.threadCount = 5
        #self.url = "https://dl.dropboxusercontent.com/u/543382/design-philosophy-sigcomm-88.pdf"
        #self.dir = ''
        #if not os.path.exists(self.dir):
        #    os.makedirs(self.dir)
        self.parse_arguments()

    def parse_arguments(self):
        ''' parse arguments, which include '-i' for input file and
        '-d' for download directory'''
        parser = argparse.ArgumentParser(prog='download accelerator', description='A simple script that downloads a single file from a URL using multithreaded downloading', add_help=True)
        parser.add_argument('-n', '--numthreads', type=int, action='store', help='specify the number of threads',default=5)
        parser.add_argument("url")
        args = parser.parse_args()
        self.threadCount = args.numthreads
        self.url = args.url

    def download(self):
        filename =  self.url.split('/')[-1].strip()
        threads = []
        r = requests.head(self.url)
        #print r
        #print r.status_code
        if r.status_code != requests.codes.ok:
            print "error: response code not ok. response code was: ", r.status_code
            return
        size = -1;
        if "content-length" in r.headers:
            size = r.headers["content-length"]
        else:
            print "no content-length header, exit program"
            return
        ranges = split(int(size),self.threadCount)
        
        global content 
        content = [""]*self.threadCount
        start = time.time()
        for i in range(0,self.threadCount):
            #content.append("hi");
            d = DownThread(self.url,filename, i, ranges[i][0], ranges[i][1])
            threads.append(d)
        #print content
        for t in threads:
            t.start()
        for t in threads:
            t.join()
        end = time.time()
        timeToDownload = end - start

        #print content
        #print "-----------------\n"
        #print "".join(content)

        #with open(filename, 'wb') as f:
        #        f.write("".join(content))

        print self.url," ",self.threadCount, " ", size, " ", timeToDownload

''' Use a thread to download one file given by url and stored in filename'''
class DownThread(threading.Thread):
    def __init__(self,url,filename, id, startByte, endByte):
        self.url = url
        self.filename = filename
        self.id = id;
        if id != 0:
            self.startByte = startByte + 1
        else:
            self.startByte = startByte
        self.endByte = endByte
        threading.Thread.__init__(self)
        self._content_consumed = False

    def run(self):
        #print 'Downloading %s' % self.url
        #'Range': 'bytes=0-299'
        #byteRangePrint = "%d: bytes=%d-%d" % (self.id, self.startByte,self.endByte)
        #print byteRangePrint
        byteRange = "bytes=%d-%d" % (self.startByte,self.endByte)
        #print "threadId :",self.id
        headers = {'Range': byteRange}
        r = requests.get(self.url, stream=True, headers=headers) #set bytes to range given
        global content
        content[self.id] = r.content
 
def split(size, n):
    #print "size:",size, "num: ", n
    k, m = size / n, size % n
    ranges = []
    for i in xrange(n):
        ranges.append((i * k + min(i, m),(i + 1) * k + min(i + 1, m)))
    #print ranges
    return ranges

if __name__ == '__main__':
    d = Downloader()
    d.download()
