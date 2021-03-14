CFLAGS=-Wall -O3 -Wextra

CC=gcc -c
MAKEDEPEND=gcc -M $(CPPFLAGS)
LINK=g++ $(LDFLAGS) -lasound -lm -lopus -lsystemd

OBJS= capture.o event_loop.o sender.o logging.o main.o \
      udp_client.o rtp_client.o session.o ip_address.o \
      sound.o rtcp_server.o

.PHONY: all
all: sound_relay

sound_relay: $(OBJS)
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
