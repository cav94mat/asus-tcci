include ./config.mk
INSTALL = install

all: asus-tcci

asus-tcci: asus-tcci.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

install:
	$(STRIP) asus-tcci
	$(INSTALL) -d $(INSTALLDIR)/usr/sbin 
	$(INSTALL) asus-tcci $(INSTALLDIR)/usr/sbin

clean:
	-rm -f asus-tcci *.o

