#include "lua.h"
#include "lauxlib.h"

#include <sys/stat.h>
#include <mqueue.h>
#define _XOPEN_SOURCE 600  
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MQD_TYPENAME "mqd_t"
#include "lua-mq.h"

static const struct { char c; mode_t b; } M[] =
{
  {'r', S_IRUSR}, {'w', S_IWUSR}, {'x', S_IXUSR},
  {'r', S_IRGRP}, {'w', S_IWGRP}, {'x', S_IXGRP},
  {'r', S_IROTH}, {'w', S_IWOTH}, {'x', S_IXOTH},
};

static int get_mode(mode_t *mode, const char* modestr)
{
  int i;

  *mode = 0;
  for (i=0; i<9; i++) {
    if(*modestr == 0) break;
    if (*modestr == M[i].c)
      *mode |= M[i].b;
    else if (*modestr == '-')
      *mode &= ~M[i].b;
    else if (*modestr == 's') {
      switch(i) {
      case 2: *mode |= S_ISUID | S_IXUSR; break;
      case 5: *mode |= S_ISGID | S_IXGRP; break;
      default: return -1; break;
      }
    }
    modestr++;
  }
  return 0;
}

static int get_oflags(const char* flagstr)
{
  if(!strcmp(flagstr, "ro"))
    return O_RDONLY;

  if(!strcmp(flagstr, "wo"))
    return O_WRONLY;

  return O_RDWR;
}

static void push_errno(lua_State *L)
{
  char msg[255];
  int ret;

  ret = strerror_r(errno, msg, sizeof(msg));
  if(!ret)
    lua_pushstring(L, msg);
  else
    lua_pushstring(L, "unknown error");
}

static int l_mq_create(lua_State *L)
{
  mqd_t id, *ptr;
  int flags;
  mode_t mode;

  flags = get_oflags(luaL_optstring(L, 2, ""));
  mode = get_mode(luaL_optstring(L, 3, ""));
  id = mq_open(luaL_checkstring(L, 1), flags | O_CREAT, mode, NULL);
  if(id == (mqd_t)-1) {
    lua_pushnil(L);
    push_errno(L);
    return 2;
  }
  ptr = (mqd_t*)lua_newuserdata(L, sizeof(mqd_t));
  *ptr = id;
  luaL_newmetatable(L, MQD_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int l_mq_open(lua_State *L)
{
  mqd_t id, *ptr;
  int flags;
  flags = get_oflags(luaL_optlstring(L, 2, "", NULL));
  id = mq_open(luaL_checkstring(L, 1), flags);
  if(id == (mqd_t)-1) {
    lua_pushnil(L);
    push_errno(L);
    return 2;
  }
  ptr = (mqd_t*)lua_newuserdata(L, sizeof(mqd_t));
  *ptr = id;
  luaL_newmetatable(L, MQD_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

static int l_mq_send(lua_State *L)
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
    push_errno(L);
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static int l_mq_receive(lua_State *L)
{
  mqd_t* id;
  unsigned int prio;
  struct mq_attr attr;
  ssize_t len;
  char* str;

  id = luaL_checkudata(L, 1, MQD_TYPENAME);
  if(mq_getattr(*id, &attr)) {
    lua_pushnil(L);
    push_errno(L);
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
    push_errno(L);
    return 2;
  }

  lua_pushlstring(L, str, len);
  free(str);
  lua_pushinteger(L, prio);
  return 2;
}

static int l_mq_close(lua_State *L)
{
  mqd_t* id;
  id = luaL_checkudata(L, 1, MQD_TYPENAME);
  if(mq_close(*id)) {
    lua_pushnil(L);
    push_errno(L);
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static int l_mq_unlink(lua_State *L)
{
  if(mq_unlink(luaL_checkstring(L, 1))) {
    lua_pushnil(L);
    push_errno(L);
    return 2;
  }

  lua_pushboolean(L, 1);
  return 1;
}

static const struct luaL_reg mq_funcs [] = {
  { "create", l_mq_create },
  { "open", l_mq_open },
  { "send", l_mq_send },
  { "receive", l_mq_receive },
  { "close", l_mq_close },
  { "unlink", l_mq_unlink },
  { NULL, NULL }
};

LUALIB_API int luaopen_mq(lua_State *L)
{
  luaL_register(L, "mq", mq_funcs);
  return 1;
}
