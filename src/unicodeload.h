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
#include <stddef.h>
typedef void *HUNICODEDATA;
typedef struct CODEINFO_impl *CODEINFO;
typedef struct _TMPBUF TMPBUF;

#ifdef __cplusplus
extern "C" {
#else
#include <uchar.h>
#endif
DLL_EXPORT HUNICODEDATA STDCALL get_default_unicodedata();
DLL_EXPORT HUNICODEDATA STDCALL get_default_unicodedata_withpath(const char *binpath, const char *txtpath);
DLL_EXPORT HUNICODEDATA STDCALL unicodedata_from_binary(const char *filepath);
DLL_EXPORT HUNICODEDATA STDCALL unicodedata_from_text(const char *filepath);
#ifdef _WIN32
DLL_EXPORT HUNICODEDATA STDCALL unicodedata_from_binaryW(const wchar_t *filepath);
DLL_EXPORT HUNICODEDATA STDCALL unicodedata_from_textW(const wchar_t *filepath);
#endif

DLL_EXPORT int STDCALL get_codeinfo(HUNICODEDATA data, char32_t code, CODEINFO *pinfo);

DLL_EXPORT char32_t get_codepoint(CODEINFO point);
DLL_EXPORT const char *STDCALL get_charname(CODEINFO point);
DLL_EXPORT const char *STDCALL get_category(CODEINFO point);
DLL_EXPORT unsigned int STDCALL get_ccc(CODEINFO point);
DLL_EXPORT const char32_t *STDCALL get_decompsition(CODEINFO point, size_t *size);
DLL_EXPORT const char *STDCALL get_decompsition_attribute(CODEINFO point);
DLL_EXPORT const char *STDCALL get_bidiclass(CODEINFO point);
DLL_EXPORT const char *STDCALL get_east_asian_wides(CODEINFO point);
DLL_EXPORT int STDCALL is_mirrored(CODEINFO point);
DLL_EXPORT int STDCALL get_numeric_digit(CODEINFO point);
DLL_EXPORT int STDCALL get_numeric_decimal(CODEINFO point);
DLL_EXPORT double STDCALL get_numeric_number(CODEINFO point);
DLL_EXPORT const char *STDCALL get_numeric_number_str(CODEINFO point);
DLL_EXPORT const char *STDCALL get_u8str(CODEINFO point, size_t *size);
DLL_EXPORT void clean_codeinfo(CODEINFO *pinfo);

DLL_EXPORT void STDCALL release_unicodedata(HUNICODEDATA f);

DLL_EXPORT int STDCALL save_unicodedata_as_binary(HUNICODEDATA data, const char *filename);
DLL_EXPORT int STDCALL load_text_and_save_binary(const char *txtfile, const char *binfile);

#ifdef _WIN32
DLL_EXPORT int STDCALL save_unicodedata_as_binaryW(HUNICODEDATA data, const wchar_t *filename);
DLL_EXPORT int STDCALL load_text_and_save_binaryW(const wchar_t *txtfile, const wchar_t *binfile);
#endif

#ifdef __cplusplus
}
#endif