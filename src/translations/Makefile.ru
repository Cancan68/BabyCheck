## Makefile for BabyCheck

VERSION 	= 2.4
TARGET 		= babycheck
APPNAME 	= "BabyCheck"
APPID 		= "CanC"

SRC = babycheck.c

CC	= m68k-palmos-gcc
CPP	= cpp
CFLAGS	= -Wall -O2 -Wno-switch

PILRC 	 = pilrc -q -L $(LANG) -Fc
OBJRES 	 = m68k-palmos-obj-res
BUILDPRC = build-prc

all:
	cp lang$(LANG).h lang.h
	$(CPP) $(TARGET).rcp.in -o $(TARGET).rcp~ -P
	sed 's/##VERSION##/$(VERSION)/g' <$(TARGET).rcp~ >$(TARGET).rcp
	$(PILRC) $(TARGET).rcp
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
	$(OBJRES) $(TARGET)
	$(BUILDPRC) $(TARGET)$(LANG).prc $(APPNAME) $(APPID) *.grc *.bin
	rm -f *~ *.o $(TARGET) *.bin *.grc *.rcp lang.h

clean:
	rm -f *~ *.o $(TARGET) *.bin *.grc *.rcp  
