#pragma once

#include <JuceHeader.h>

// Include the system Lua 5.4 headers.  These are found via the Lua include
// directory added by find_package(Lua) in CMakeLists.txt.
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

// Sanity check: refuse to compile against Lua < 5.4.
#if LUA_VERSION_NUM < 504
 #error "protoplug requires Lua 5.4 or later.  Please install lua5.4-devel (Fedora) or equivalent."
#endif

namespace protolua
{

// Expose the real Lua types inside the protolua namespace so that
// LuaLink.cpp (which uses protolua::lua_State*, protolua::lua_Number, etc.)
// continues to compile without modification.
using lua_State    = ::lua_State;
using lua_Number   = ::lua_Number;
using lua_CFunction = ::lua_CFunction;

// Thin RAII wrapper around lua_State* that keeps the same public interface
// as the old dynamic-loading LuaState so that LuaLink.cpp is unchanged.
class LuaState
{
public:
    // defaultDir is kept for API compatibility; it is ignored when Lua is
    // linked statically.
    explicit LuaState (juce::File defaultDir);
    ~LuaState();

    void        openlibs();
    int         loadbuffer (const char* buff, size_t sz, const char* name);
    int         loadstring (const char* s);
    const char* tolstring  (int idx, size_t* len);
    lua_Number  tonumber   (int idx);
    int         toboolean  (int idx);
    void        pushcclosure (lua_CFunction fn, int n);
    void        close();
    int         gettop();
    void        settop (int idx);
    int         pcall (int nargs, int nresults, int errfunc);
    void        getfield (int idx, const char* k);
    void        pushvalue (int idx);
    void        pushlightuserdata (void* p);
    void        pushstring (const char* s);
    void        pushnumber (lua_Number n);
    void        pushboolean (int b);
    int         type (int idx);
    void        setfield (int idx, const char* k);
    int         isstring  (int idx);
    int         isnumber  (int idx);
    const char* ltypename (int t);
    void*       newuserdata (size_t sz);

    // Convenience helpers — same interface as before.
    void setglobal (const char* n)  { lua_setglobal (l, n); }
    void getglobal (const char* n)  { lua_getglobal (l, n); }
    void pop (int n)                { lua_settop (l, -(n) - 1); }
    const char* tostring (int idx)  { return tolstring (idx, nullptr); }
    bool isfunction (int idx)       { return lua_type (l, idx) == LUA_TFUNCTION; }
    bool isboolean  (int idx)       { return lua_type (l, idx) == LUA_TBOOLEAN;  }

    lua_State*   l       = nullptr;
    bool         failed  = false;
    juce::String errmsg;
};

} // namespace protolua
