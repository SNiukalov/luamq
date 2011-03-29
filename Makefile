CONFIG= ./config

include $(CONFIG)

OBJS= src/lua-mq.o
SRCS= src/lua-mq.h src/lua-mq.c
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
