# Makefile to make it easy to switch between release and debug targets

CFLAGS :=
PROG := prog

all: $(PROG)

debug: CFLAGS +=
debug: $(PROG)

opt: CFLAGS +=
opt: $(PROG)

release: CFLAGS += -D NDEBUG
release: opt

$(PROG):
	pebble build

clean:
	pebble clean
