# Makefile for echo client and server

CXX=			g++ $(CCFLAGS)

MESSAGE-SERVER=		msgd.o 
MESSAGE-CLIENT=		msg.o
OBJS =			$(MESSAGE-SERVER) $(MESSAGE-CLIENT)

LIBS=

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
