CONTIKI_PROJECT = encryption_and_transmission
all: $(CONTIKI_PROJECT)

CONTIKI = ../..
#MAKE_NET = MAKE_NET_NULLNET
MODULES += os/net/nullnet
include $(CONTIKI)/Makefile.include
