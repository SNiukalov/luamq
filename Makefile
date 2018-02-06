CONFIG= ./config

include $(CONFIG)

OBJS= src/lua-mq.o
SRCS= src/lua-mq.h src/lua-mq.c
AR= ar rcu
RANLIB= ranlib

lib: src/$(LIBNAME)

src/$(LIBNAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIB_OPTION)

install:
	mkdir -p $(DESTDIR)$(LUA_LIBDIR)
	cp src/$(LIBNAME) $(DESTDIR)$(LUA_LIBDIR)/

clean:
	rm -f src/$(LIBNAME) src/*.o
