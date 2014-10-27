
import time
import os
import socket
import errno


"""
WebConfig is an object used to parse and store the contents 
"""
class HTTPServer:

    def __init__(self,debug):
        self.hosts = {}
        self.media = {}
        self.parameters = {}
        self.debug = debug
        #Constants
        self.EOM = "\r\n"
        self.HTTP_OK                 = "HTTP/1.1 200 OK \r\n"
        self.HTTP_PARTIAL            = "HTTP/1.1 206 Partial Content \r\n"
        self.HTTP_BAD_REQUEST        = "HTTP/1.1 400 Bad Request \r\n"
        self.HTTP_NOT_FOUND          = "HTTP/1.1 404 Not Found \r\n" 
        self.HTTP_FORBIDDEN          = "HTTP/1.1 403 Forbidden \r\n"
        self.HTTP_SERVER_ERROR       = "HTTP/1.1 500 Internal Server Error \r\n"
        self.HTTP_NOT_IMPLEMENTED    = "HTTP/1.1 501 Not Implemented \r\n"

        self.HTTP_BAD_REQUEST_MSG        = "<!DOCTYPE html><html><head><title>400 Error</title></head><body>400: Bad Request</body></html>"
        self.HTTP_NOT_FOUND_MSG          = "<!DOCTYPE html><html><head><title>404 Error</title></head><body>404: Page Not Found Error</body></html>" 
        self.HTTP_FORBIDDEN_MSG          = "<!DOCTYPE html><html><head><title>403 Error</title></head><body>403: Forbidden</body></html>"
        self.HTTP_SERVER_ERROR_MSG       = "<!DOCTYPE html><html><head><title>500 Error</title></head><body>400: Internal Server Error</body></html>"
        self.HTTP_NOT_IMPLEMENTED_MSG    = "<!DOCTYPE html><html><head><title>501 Error</title></head><body>501: Not Implemented</body></html>"

        self.DEFAULT = "default"
        self.MEDIA = "media"
        self.HOST = "host"
        self.PARAMETER = "parameter"
        self.TIMEOUT = "timeout"

    """
    Parses the conf file to initialize the server
    """
    def parseConfig(self,filename):
        try:
            with open(filename, "r") as f:
                for line in f:
                    line = line.lower()
                    tokens = line.split()
                    if len(tokens) < 3:
                        continue
                    command = tokens[0]
                    if command == self.HOST:
                        self.hosts[tokens[1]] = tokens[2]
                    elif command == self.MEDIA:
                        self.media[tokens[1]] = tokens[2]
                    elif command == self.PARAMETER:
                        self.parameters[tokens[1]] = tokens[2]
            return True
        except IOError:
            return False

    """
    handleHTTPMessage
    Responds to HTTP messages from clients.
    Parses and determines which request type and calls the appropriate function
    """
    def handleHTTPMessage(self, message, client, cache):
        self.log("message: %s" % message)

        lines = message.lstrip().split("\r\n")
        respHeaders = self.basicHeaders(self.HTTP_BAD_REQUEST_MSG)
        if len(lines) < 1:
            self.log("Error: message is empty: %s " % message)
            self.send_response(client, self.HTTP_BAD_REQUEST, 
                                respHeaders, self.HTTP_BAD_REQUEST_MSG)
            return
        statusLine = lines[0].split()
        if not self.checkStatusLine(statusLine):
            return self.send_response(client, self.HTTP_BAD_REQUEST, 
                                respHeaders, self.HTTP_BAD_REQUEST_MSG)
        uri = statusLine[1]
        headers = self.parseHeaders(lines[1:])
        if statusLine[0] == "GET":
            self.processGetRequest(headers, uri, client, cache, True)
        elif statusLine[0] == "HEAD":
            self.processGetRequest(headers, uri, client, cache, False)
        else:
            self.log("Error: not implemented request type: %s" % statusLine[0])
            self.send_response(client, self.HTTP_NOT_IMPLEMENTED, 
                self.basicHeaders(self.HTTP_NOT_IMPLEMENTED_MSG), self.HTTP_NOT_IMPLEMENTED_MSG)

    def checkStatusLine(self, statusLine):
        if len(statusLine) < 3:
            self.log("Error: status line too short! : %s" % statusLine)
            return False
        if statusLine[2] != "HTTP/1.1":
            iself.log("Error: Bad version request: %s" % statusLine[2])
            return False
        return True

    def parseHeaders(self, lines):
        headers = {}
        for l in lines:
            tok = l.split(":")
            headers[tok[0].strip()] = tok[1].strip()
        return headers

    def stringifyHeaders(self, headers):
        returnString = ""
        for key in headers.keys():
            returnString += key +":"+ headers[key] + "\r\n"
        return returnString


    """
    processGetRequest
    processes a GET request and responds.
    handles request headers
    determines response headers
    sends response and content
    Also handles HEAD requests determined by the sendFile parameter
    """
    def processGetRequest(self, reqHeaders, uri, client, cache, sendFile):
        (start, end, code) = self.parseRangeHeader(reqHeaders)
        (ok,filename) = self.determineFilename(reqHeaders,uri)
        if not ok:
            respHeaders = self.basicHeaders(self.HTTP_SERVER_ERROR_MSG)
            self.send_response(client, self.HTTP_SERVER_ERROR, 
                respHeaders, self.HTTP_SERVER_ERROR_MSG)

        contentType = self.determineFileType(filename)
        respHeaders = self.basicHeaders("")
        respHeaders["Content-Type"] = contentType

        try :
            with open (filename) as f:
                fileStat = os.stat(filename)
                size = fileStat.st_size
                lastModTime = self.get_time(fileStat.st_mtime)
                respHeaders["Last-Modified"] = lastModTime
                if not sendFile:
                    respHeaders["Content-Length"] =  "0"
                elif start != -1 and end - start < size:
                    respHeaders["Content-Length"] =  str(end - start)
                else:
                    respHeaders["Content-Length"] =  str(size)
                    
                self.log("size: %d" % size)
                self.log("content Length: %s" % respHeaders["Content-Length"])
                
                self.send_response(client, code, respHeaders, "")
                if sendFile:
                    self.send_file(client, f, start, end)
             
        except IOError as ( errno , strerror ):
            if errno == 13:
                respHeaders = self.basicHeaders(self.HTTP_FORBIDDEN_MSG)
                self.send_response(client, self.HTTP_FORBIDDEN, 
                            respHeaders, self.HTTP_FORBIDDEN_MSG)
                self.log("403 : %s" % filename)
            elif errno == 2:
                respHeaders = self.basicHeaders(self.HTTP_NOT_FOUND_MSG)
                self.send_response(client, self.HTTP_NOT_FOUND, 
                            respHeaders, self.HTTP_NOT_FOUND_MSG)
                self.log("404 : %s" % filename)
            else:
                respHeaders = self.basicHeaders(self.HTTP_SERVER_ERROR_MSG)
                self.send_response(client, self.HTTP_SERVER_ERROR, 
                            respHeaders, self.HTTP_SERVER_ERROR_MSG)
                self.log("500 : %s" % filename)

    """
    parseRangeHeader -
    Looks for a range headers and parses that requests 
    returns (start, end, responseCode) based on that headers
    if no range header is found then it returns -1 for start and end 
    and HTTP_OK for code
    """
    def parseRangeHeader(self,reqHeaders):
        start = -1
        end = -1
        code = self.HTTP_OK
        if "Range" in reqHeaders.keys():
            rangeHeader = reqHeaders["Range"]
            self.log("Range: %s" %rangeHeader)
            rangeList = rangeHeader.split("=")
            self.log(rangeList)
            if len(rangeList) == 2:
                rangeList = rangeList[1].split("-")
                self.log(rangeList)
                if len(rangeList) == 2:
                    start = int(rangeList[0])
                    end = int(rangeList[1]) + 1
                    self.log("start: %d" % start)
                    self.log("end: %d" % end)
                    code = self.HTTP_PARTIAL
        return (start, end, code)

    """
    determineFilename
    Uses the host header and uri to determine the filename
    returns (ok,filename)
    """
    def determineFilename(self, reqHeaders,uri):
        if uri == "/":
            uri = "/index.html"
        host = ""

        if "Host" in reqHeaders.keys():
            host = reqHeaders["Host"]
        hostDir = ""
        if host in self.hosts.keys():
            hostDir = self.hosts[host]
        elif self.DEFAULT in self.hosts.keys():
            hostDir = self.hosts[self.DEFAULT]
        else:
            self.log("default not found in hosts, check config file")
            return (False, "")

        filename = hostDir + uri
        return (True,filename)

    def determineFileType(self, filename):
        index = filename.rfind(".")
        contentType = "text/plain"
        if index != -1:
            extension = filename[index+1:]
            if extension in self.media:
                contentType = self.media[extension]
        return contentType

    
                
    def basicHeaders(self, content):
        respHeaders = {}
        respHeaders["Server"] = "MyServer!/1.0"
        now = time.time()
        respHeaders["Date"] = self.get_time(now)
        respHeaders["Content-Length"] =  str(len(content))
        respHeaders["Content-Type"] = "text/html"
        return respHeaders



    def send_response(self,client, response, headers, content):
        self.log("sending: %s" % response)
        msg = response + self.stringifyHeaders(headers) + self.EOM + content
        client.sendall(msg)

    def send_file(self, client, f, start, end):
        content = ""
        if start != -1:
            f.seek(start)
        sentTotal = 0
        #loop until entire file is sent
        while True: 
            content = f.read(10000)
            if not content:
                self.log("finished send")
                break
            
            if start != -1:
                whatsLeft = end - start - sentTotal
                if whatsLeft == 0:
                    self.log("finished partial send")
                    break
                elif len(content) > whatsLeft:
                    content = content[:whatsLeft]
                    self.log("trim the value, %d" % whatsLeft)
            numSent = 0
            #loop until current file chunk is sent
            while numSent < len(content):
                try:
                    s = client.send(content[numSent:])
                except socket.error, (value,message):
                    if value == errno.EWOULDBLOCK or value == errno.EAGAIN:
                        self.log("error in send: %d, %d, %d" % (value, numSent, len(content)))
                        self.log(message)
                        continue
                numSent += s
            sentTotal += numSent
        
    def log(self,s):
        if self.debug:
            print s

    def get_time(self, t):
        gmt = time.gmtime(t)
        format = '%a, %d %b %Y %H:%M:%S GMT'
        time_string = time.strftime(format,gmt)
        return time_string













