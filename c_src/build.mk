CFLAGS=-Wall -O3 -Wextra

CC=gcc -c
MAKEDEPEND=gcc -M $(CPPFLAGS)
LINK=g++ $(LDFLAGS) -lasound -lm -lopus -lsystemd

sr_OBJS= socket.o capture.o event_loop.o sender.o logging.o sound_relay_main.o \
      udp_connection.o rtp_client.o session.o ip_address.o \
      sound.o rtcp_server.o rtp_server.o rtcp_client.o replay.o

pr_OBJS= socket.o replay.o event_loop.o logging.o play_rtp_main.o \
      udp_connection.o rtp_server.o session.o ip_address.o \
	  sound.o rtcp_client.o rtp_client.o rtcp_server.o capture.o


.PHONY: all
all: sound_relay play_rtp

sound_relay: $(sr_OBJS)
	$(LINK) $(sr_OBJS) -o $@

play_rtp : $(pr_OBJS)
	$(LINK) $(pr_OBJS) -o $@

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


-include $(sr_OBJS:.o=.d)
-include $(pr_OBJS:.o=.d)
