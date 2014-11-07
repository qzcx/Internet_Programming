import requests
import os
import time

def GetServiceSpeed(url, outputFile, results):
    f = open(outputFile,'a')
    start = time.time()
    requests.get(url, stream=True)
    end = time.time()
    dif = end - start

    #f.write(str(dif) + "\n" )
    results.append(dif)

def RunNTimes(n):
    GetServiceSpeed("http://localhost:3000/file005.txt", "./ServiceTest.txt", [])
    results = []
    for i in range(0,n):  
        GetServiceSpeed("http://localhost:3000/file005.txt", "./ServiceTest.txt", results)
    ans = sum(results)/float(len(results))
    print ans
    print 1/ans

if __name__ == '__main__':
    RunNTimes(100)
