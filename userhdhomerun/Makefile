default_target: all

build/Makefile:
	@-mkdir build 2>/dev/null
	cd build; cmake ..

clean:
	rm -rf build

run: all
	build/userhdhomerun

all: build/Makefile
	$(MAKE) -C build $(MAKE_FLAGS) $@
