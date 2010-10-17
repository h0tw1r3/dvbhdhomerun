CMAKETWEAKS = cd build; cmake ..

ifneq ($(CFLAGS),)
CMAKETWEAKS +=  -DCMAKE_C_FLAGS="$(CFLAGS)"
endif

ifneq ($(LDFLAGS),)
CMAKETWEAKS += -DCMAKE_EXE_LINKER_FLAGS:STRING="$(LDFLAGS)" -DCMAKE_MODULE_LINKER_FLAGS:STRING="$(LDFLAGS)" -DCMAKE_SHARED_LINKER_FLAGS:STRING="$(LDFLAGS)"
endif

ifneq ($(PREFIX),)
CMAKETWEAKS += -DCMAKE_INSTALL_PREFIX="$(PREFIX)"
endif

CMAKETWEAKS += -DCMAKE_BUILD_TYPE=DEBUG

default_target: all

build/Makefile:
	@-mkdir build 2>/dev/null
	$(CMAKETWEAKS)

clean:
	rm -rf build

install: build/Makefile
	$(CMAKETWEAKS)
	$(MAKE) -C build $(MAKE_FLAGS) $@ install

run: all
	build/userhdhomerun

all: build/Makefile
	$(MAKE) -C build $(MAKE_FLAGS) $@
