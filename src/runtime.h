#pragma once

#ifdef _WIN32
#ifndef DLL_EXPORT
#define DLL_EXPORT __declspec(dllimport)
#endif
#define STDCALL __stdcall
#else
#ifdef DLL_EXPORT
#undef DLL_EXPORT
#endif
#define DLL_EXPORT
#define STDCALL
#endif

#ifdef __cplusplus
#include <coutwrapper.h>
DLL_EXPORT extern commonlib2::CinWrapper &Cin;
DLL_EXPORT extern commonlib2::CoutWrapper &Cout;
DLL_EXPORT extern commonlib2::CoutWrapper &Clog;
#endif

#define USE_CALLBACK 0

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __EMSCRIPTEN__
#if USE_CALLBACK
extern void unicode_callback(const char *ptr, size_t size);
size_t EMSCRIPTEN_KEEPALIVE get_result_text_len();
const char *EMSCRIPTEN_KEEPALIVE get_result_text();
int EMSCRIPTEN_KEEPALIVE command_callback(const char *str);
#endif
#else
#define EMSCRIPTEN_KEEPALIVE
#endif
DLL_EXPORT int STDCALL EMSCRIPTEN_KEEPALIVE command_str(const char *str, int onlybuf);
DLL_EXPORT int STDCALL EMSCRIPTEN_KEEPALIVE start_load();
DLL_EXPORT int STDCALL command_argv(int argc, char **argv);
DLL_EXPORT int STDCALL init_io(int sync, const char **err);

DLL_EXPORT int STDCALL runtime_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif
