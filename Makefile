CONFIG= ./config

include $(CONFIG)

OBJS= src/luamq.o
SRCS= src/luamq.h src/luamq.c
AR= ar rcu
RANLIB= ranlib

lib: src/$(LIBNAME)

src/$(LIBNAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LIB_OPTION) $(OBJS)

install:
	mkdir -p $(LUA_LIBDIR)
	cp src/$(LIBNAME) $(LUA_LIBDIR)/

clean:
	rm -f src/$(LIBNAME) src/*.o
