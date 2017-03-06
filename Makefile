# Makefile to make it easy to switch between release and debug targets

OPTIONS :=
PROG := prog

all: $(PROG)

debug: OPTIONS +=
debug: $(PROG)

opt: OPTIONS +=
opt: $(PROG)

release: OPTIONS += --release
release: opt

$(PROG):
	pebble build options $(OPTIONS)

clean:
	pebble clean
