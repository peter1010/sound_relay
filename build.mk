CFLAGS=-Wall -O3 -Wextra

CC=gcc -c
MAKEDEPEND=gcc -M $(CPPFLAGS)
LINK=g++ $(LDFLAGS) -lasound -lm

OBJS= capture.o event_loop.o sender.o logging.o main.o rtsp_server.o tcp_server.o \
      tcp_connection.o rtsp_connection.o network.o connection.o udp_client.o \
      udp_connection.o rtp_connection.o rtp_client.o media.o session.o ip_address.o 

.PHONY: all
all: capture

capture: $(OBJS)
	$(LINK) $(OBJS) -o $@


%.o : %.c
	$(CC) $(CPPFLAGS) -MMD $(CFLAGS) -o $@ $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@rm -f $*.d
	@mv $*.P $*.d
	
%.o : %.cpp
	$(CC) $(CPPFLAGS) -MMD $(CFLAGS) -o $@ $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@rm -f $*.d
	@mv $*.P $*.d


-include $(OBJS:.o=.d)
