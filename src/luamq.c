#include "lua.h"
#include "lauxlib.h"

#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MQD_TYPENAME "mqd_t"

LUALIB_API int luaopen_luamq(lua_State *L);

static int get_oflags(const char* flagstr)
{
  if(!strcmp(flagstr, "ro"))
    return O_RDONLY;

  if(!strcmp(flagstr, "wo"))
    return O_WRONLY;

  return O_RDWR;
}

static int l_luamq_create(lua_State *L)
{
  mqd_t id, *ptr;
  int flags;
  flags = get_oflags(luaL_optlstring(L, 2, "", NULL));
  id = mq_open(luaL_checkstring(L, 1), flags | O_CREAT, S_IRWXU | S_IRWXG, NULL);
  if(id == (mqd_t)-1) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }
  ptr = (mqd_t*)lua_newuserdata(L, sizeof(mqd_t));
  *ptr = id;
  luaL_newmetatable(L, MQD_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int l_luamq_open(lua_State *L)
{
  mqd_t id, *ptr;
  int flags;
  flags = get_oflags(luaL_optlstring(L, 2, "", NULL));
  id = mq_open(luaL_checkstring(L, 1), flags);
  if(id == (mqd_t)-1) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }
  ptr = (mqd_t*)lua_newuserdata(L, sizeof(mqd_t));
  *ptr = id;
  luaL_newmetatable(L, MQD_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int l_luamq_send(lua_State *L)
{
  mqd_t* id;
  unsigned int prio;
  size_t len;
  const char* str;

  id = luaL_checkudata(L, 1, MQD_TYPENAME);
  str = luaL_checklstring(L, 2, &len);
  prio = luaL_optint(L, 3, 0);
  if(mq_send(*id, str, len, prio)) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static int l_luamq_receive(lua_State *L)
{
  mqd_t* id;
  unsigned int prio;
  struct mq_attr attr;
  ssize_t len;
  char* str;

  id = luaL_checkudata(L, 1, MQD_TYPENAME);
  if(mq_getattr(*id, &attr)) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }

  len = attr.mq_msgsize;
  str = malloc(len);
  if(!str) {
    lua_pushnil(L);
    lua_pushstring(L, "bad alloc");
    return 2;
  }

  len = mq_receive(*id, str, len, &prio);
  if(len < 0) {
    free(str);
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }

  lua_pushlstring(L, str, len);
  free(str);
  lua_pushinteger(L, prio);
  return 2;
}

static int l_luamq_close(lua_State *L)
{
  mqd_t* id;
  id = luaL_checkudata(L, 1, MQD_TYPENAME);
  if(mq_close(*id)) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static int l_luamq_unlink(lua_State *L)
{
  if(mq_unlink(luaL_checkstring(L, 1))) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno)); /* TODO: thread safety */
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static const struct luaL_reg luamq_funcs [] = {
  { "create", l_luamq_create },
  { "open", l_luamq_open },
  { "send", l_luamq_send },
  { "receive", l_luamq_receive },
  { "close", l_luamq_close },
  { "unlink", l_luamq_unlink },
  { NULL, NULL }
};

LUALIB_API int luaopen_luamq(lua_State *L)
{
  luaL_register(L, "luamq", luamq_funcs);
  return 1;
}
