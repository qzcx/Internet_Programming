# Makefile for echo client and server

CXX=			g++ $(CCFLAGS)

MESSAGE-SERVER=		util.o msgd.o msg_server.o server.o
MESSAGE-CLIENT=		util.o msg.o msg_client.o client.o
OBJS =			$(MESSAGE-SERVER) $(MESSAGE-CLIENT)

LIBS=	-pthread	

CCFLAGS= -g

all:	message-server message-client

message-server:$(MESSAGE-SERVER)
	$(CXX) -o server $(MESSAGE-SERVER) $(LIBS)

message-client:$(MESSAGE-CLIENT)
	$(CXX) -o client $(MESSAGE-CLIENT) $(LIBS)

clean:
	rm -f $(OBJS) $(OBJS:.o=.d)

realclean:
	rm -f $(OBJS) $(OBJS:.o=.d) server client


# These lines ensure that dependencies are handled automatically.
%.d:	%.cc
	$(SHELL) -ec '$(CC) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

include	$(OBJS:.o=.d)
