COMPILER = pebble
PHONE = 10.137.135.98
SOURCE = src
SRCTYPE = c
HDRTYPE = h
BINARY = build

all: $(BINARY)

$(BINARY): $(SOURCE)/*.$(SRCTYPE) $(SOURCE)/*.$(HDRTYPE)
	$(COMPILER) build

clean:
	rm -r $(BINARY)/*

install: $(BINARY)
	$(COMPILER) install --phone $(PHONE)

