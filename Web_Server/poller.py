import errno
import select
import socket
import sys
import traceback
import time

class Poller:
    """ Polling server """
    def __init__(self,port, webParse):
        self.host = ""
        self.port = port
        self.open_socket()
        self.clients = {}
        self.caches = {}
        self.lastused = {}
        self.size = 1024
        self.webParse = webParse

    def open_socket(self):
        """ Setup the socket for incoming clients """
        try:
            self.serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.serverSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1)
            self.serverSocket.bind((self.host,self.port))
            self.serverSocket.listen(5)
            self.serverSocket.setblocking(0)
        except socket.error, (value,message):
            if self.serverSocket:
                self.serverSocket.close()
            print "Could not open socket: " + message
            sys.exit(1)

    def run(self):
        """ Use poll() to handle each incoming client."""
        self.poller = select.epoll()
        self.pollmask = select.EPOLLIN | select.EPOLLHUP | select.EPOLLERR
        self.poller.register(self.serverSocket,self.pollmask)

        timeout = 1;
        #print self.webParse.parameters["timeout"]
        if self.webParse.parameters["timeout"]:
            timeout = int(self.webParse.parameters["timeout"])

            self.webParse.log("timeout value is: %d" % timeout)

        self.lastChecked = time.time()

        while True:
            # poll sockets
            try: 
                fds = self.poller.poll(timeout=timeout)
            except:
                self.webParse.log("polling exception")
                return
            for (fd,event) in fds:
                # handle errors
                if event & (select.POLLHUP | select.POLLERR):
                    self.handleError(fd)
                    continue
                # handle the server socket
                if fd == self.serverSocket.fileno():
                    self.handleServer()
                    continue
                # handle client socket
                self.lastused[fd] = time.time()
                result = self.handleClient(fd)
            self.sweepSockets(timeout)

    def sweepSockets(self, timeout):
        now = time.time()
        if now - self.lastChecked > timeout * 5:
            deleteThese = []
            for fd in self.lastused.keys():
                if now - self.lastused[fd] > timeout * 5:
                   deleteThese.append(fd)
            for fd in deleteThese:
                self.webParse.log("socket timeout: %d" % fd)
                self.cleanupSocket(fd)
            self.lastChecked = time.time()

    def cleanupSocket(self,fd):
        self.poller.unregister(fd)
        if fd in self.clients:
            self.clients[fd].close()
            del self.clients[fd]
        if fd in self.caches:
            del self.caches[fd]
        if fd in self.lastused:
            del self.lastused[fd]

    def handleError(self,fd):
        self.webParse.log("in handleError")
        if fd == self.serverSocket.fileno():
            # recreate server socket
            self.poller.unregister(fd)
            self.serverSocket.close()
            self.open_socket()
            self.poller.register(self.serverSocket,self.pollmask)
        else:
            self.cleanupSocket(fd)

    def handleServer(self):
        # accept as many clients are possible
        self.webParse.log("handleServer")
        (client,address) = self.serverSocket.accept()
        self.webParse.log("acceping: %d" % client.fileno())
        client.setblocking(0)
        self.clients[client.fileno()] = client
        self.caches[client.fileno()] = ""
        self.lastused[client.fileno()] = time.time()
        self.poller.register(client.fileno(),self.pollmask)

    def handleClient(self,fd):
        #i =0;
        while True:
            try:
                data = self.clients[fd].recv(self.size)
                self.webParse.log("%d recv: %s" % (fd,data))
            except socket.error, (value,message):
                if value == errno.EAGAIN or errno.EWOULDBLOCK:
                    self.webParse.log("error in recv: %d" % value)
                    self.webParse.log(message)
                    break
                print traceback.format_exc()
                sys.exit()

            if data:
                #append to cache for that socket
                self.caches[fd] += data
                message = self.read_message(fd)
                if not message:
                    continue
                #process any HTTP messages present
                response = self.webParse.handleHTTPMessage(message.strip(), self.clients[fd], self.caches[fd])
                #self.clients[fd].sendall(response)
                
                #if messages processed, break from loop
                return
            else:
                self.cleanupSocket(fd)
                return


    def read_message(self, fd):
        #check for end of a message (\r\n\r\n)
        index = self.caches[fd].find("\r\n\r\n")
        if index == "-1":
            return None
        message = self.caches[fd][0:index+1]
        #leave any remainder in the cache
        self.caches[fd] = self.caches[fd][index+1:]
        return message

    #def send_response(self, fd, message):














