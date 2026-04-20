#include "LuaState.h"

namespace protolua
{

LuaState::LuaState (juce::File /*defaultDir — ignored, Lua is linked statically*/)
{
    l = luaL_newstate();
    if (! l)
    {
        failed = true;
        errmsg = "Failed to create Lua state (luaL_newstate returned null).";
    }
}

LuaState::~LuaState()
{
    if (l)
        close();
}

void        LuaState::openlibs()                                          { luaL_openlibs (l); }
int         LuaState::loadbuffer (const char* b, size_t sz, const char* n){ return luaL_loadbuffer (l, b, sz, n); }
int         LuaState::loadstring (const char* s)                          { return luaL_loadstring (l, s); }
const char* LuaState::tolstring  (int i, size_t* len)                     { return lua_tolstring (l, i, len); }
lua_Number  LuaState::tonumber   (int i)                                  { return lua_tonumber (l, i); }
int         LuaState::toboolean  (int i)                                  { return lua_toboolean (l, i); }
void        LuaState::pushcclosure (lua_CFunction fn, int n)              { lua_pushcclosure (l, fn, n); }
void        LuaState::close()                                             { lua_close (l); l = nullptr; }
int         LuaState::gettop()                                            { return lua_gettop (l); }
void        LuaState::settop (int i)                                      { lua_settop (l, i); }
int         LuaState::pcall (int n, int r, int e)                        { return lua_pcall (l, n, r, e); }
void        LuaState::getfield (int i, const char* k)                     { lua_getfield (l, i, k); }
void        LuaState::pushvalue (int i)                                   { lua_pushvalue (l, i); }
void        LuaState::pushlightuserdata (void* p)                         { lua_pushlightuserdata (l, p); }
void        LuaState::pushstring (const char* s)                          { lua_pushstring (l, s); }
void        LuaState::pushnumber (lua_Number n)                           { lua_pushnumber (l, n); }
void        LuaState::pushboolean (int b)                                 { lua_pushboolean (l, b); }
int         LuaState::type (int i)                                        { return lua_type (l, i); }
void        LuaState::setfield (int i, const char* k)                     { lua_setfield (l, i, k); }
int         LuaState::isstring  (int i)                                   { return lua_isstring (l, i); }
int         LuaState::isnumber  (int i)                                   { return lua_isnumber (l, i); }
const char* LuaState::ltypename (int t)                                   { return lua_typename (l, t); }
void*       LuaState::newuserdata (size_t sz)                             { return lua_newuserdata (l, sz); }

} // namespace protolua
