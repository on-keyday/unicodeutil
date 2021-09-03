#define DLL_EXPORT __declspec(dllexport)
#include "unicodeload.h"

#include <unicodedata.h>

#include <chrono>
#include <fstream>
#include <iostream>
using namespace commonlib2;

#ifndef USE_BUILTIN_BINARY
#define USE_BUILTIN_BINARY 0
#endif

struct CODEINFO_impl {
    CodeInfo *base = nullptr;
    char32_t real = ~0;
    std::string v3_str;
    std::string dummyname;
    std::string u8str;
};

UnicodeData *new_data() {
    try {
        return new UnicodeData();
    } catch (...) {
        return nullptr;
    }
}

CODEINFO_impl *new_info() {
    try {
        return new CODEINFO_impl();
    } catch (...) {
        return nullptr;
    }
}

template <class Buf>
HUNICODEDATA unicodedata_from_binary_impl_detail(Deserializer<Buf> &r) {
    if (r.eof()) {
        return nullptr;
    }
    UnicodeData *ret = new_data();
    if (!ret)
        return nullptr;
    if (!deserialize_unicodedata(r, *ret)) {
        delete ret;
        return nullptr;
    }
    return (HUNICODEDATA)ret;
}

template <class C>
HUNICODEDATA unicodedata_from_binary_impl(C *filepath) {
    Deserializer<FileReader> r(filepath);
    return unicodedata_from_binary_impl_detail(r);
}

#if USE_BUILTIN_BINARY
const char *load_builtin_unicodedata(size_t &size) {
    static const unsigned char CODE[] = {
#include "binary.csv"
    };
    size = sizeof(CODE);
    return (const char *)CODE;
}

HUNICODEDATA unicodedata_from_builtin() {
    size_t size;
    const char *data = load_builtin_unicodedata(size);
    Deserializer<Sized<const char>> r(Sized(data, size));
    return unicodedata_from_binary_impl_detail(r);
}
#endif

HUNICODEDATA STDCALL unicodedata_from_binary(const char *filepath) {
    return unicodedata_from_binary_impl(filepath);
}

template <class C>
HUNICODEDATA unicodedata_from_text_impl(C *filepath) {
    auto c = load_unicodedata_text(filepath);
    if (c.size() == 0) {
        return nullptr;
    }
    UnicodeData *ret = new_data();
    if (!ret)
        return nullptr;
    if (!parse_unicodedata(c, *ret)) {
        delete ret;
        return nullptr;
    }
    return (HUNICODEDATA)ret;
}

HUNICODEDATA STDCALL unicodedata_from_text(const char *filepath) {
    return unicodedata_from_text_impl(filepath);
}

#ifdef _WIN32
HUNICODEDATA STDCALL unicodedata_from_binaryW(const wchar_t *filepath) {
    return unicodedata_from_binary_impl(filepath);
}

HUNICODEDATA STDCALL unicodedata_from_textW(const wchar_t *filepath) {
    return unicodedata_from_text_impl(filepath);
}
#endif

HUNICODEDATA default_unicodedata(const char *binpath = "./unicodedata.bin", const char *txtpath = "./unicodedata.txt") {
    if (auto data = unicodedata_from_binary(binpath)) {
        return data;
    }
    if (auto data = unicodedata_from_text(txtpath)) {
        return data;
    }
#if USE_BUILTIN_BINARY
    return unicodedata_from_builtin();
#endif
    return nullptr;
}

bool loaded = false;

HUNICODEDATA load_default_unicodedata(const char *binpath = "./unicodedata.bin", const char *txtpath = "./unicodedata.txt") {
    static std::unique_ptr<UnicodeData> data((UnicodeData *)default_unicodedata(binpath, txtpath));
    loaded = true;
    return (HUNICODEDATA)data.get();
}

HUNICODEDATA get_default_unicodedata_impl(const char *binpath = "./unicodedata.bin", const char *txtpath = "./unicodedata.txt") {
    static HUNICODEDATA data = load_default_unicodedata(binpath, txtpath);
    return data;
}

HUNICODEDATA STDCALL get_default_unicodedata() {
    return get_default_unicodedata_impl();
}

HUNICODEDATA STDCALL get_default_unicodedata_withpath(const char *binpath, const char *txtpath) {
    return get_default_unicodedata_impl(binpath, txtpath);
}

void STDCALL release_unicodedata(HUNICODEDATA f) {
    if (loaded) {
        if (f == get_default_unicodedata_impl()) {
            return;
        }
    }
    UnicodeData *data = (UnicodeData *)f;
    delete data;
}

CodeInfo *get_codepointobj(HUNICODEDATA data, char32_t code) {
    UnicodeData *dat = (UnicodeData *)data;
    if (auto it = dat->codes.find(code); it != dat->codes.end()) {
        return &(*it).second;
    }
    for (auto &r : dat->ranges) {
        if (code >= r.begin->codepoint && code <= r.end->codepoint) {
            return r.begin;
        }
    }
    return nullptr;
}

int STDCALL get_codeinfo(HUNICODEDATA data, char32_t code, CODEINFO *pinfo) {
    if (!data || !pinfo || *pinfo)
        return 0;
    CODEINFO_impl **res = pinfo;

    CodeInfo *detail = get_codepointobj(data, code);
    if (!detail)
        return 0;

    auto ret = new_info();
    if (!ret)
        return 0;
    ret->base = detail;
    ret->real = code;
    if (code != detail->codepoint) {
        ret->dummyname = detail->name;
        ret->dummyname.erase(0, 1);
        ret->dummyname.erase(ret->dummyname.end() - 8, ret->dummyname.end());
    }
    ret->v3_str = detail->numeric.stringify();
    Reader(std::u32string(1, code)) >> u8filter >> ret->u8str;
    *res = ret;
    return 1;
}

char32_t get_codepoint(CODEINFO point) {
    if (!point)
        return ~0;
    CODEINFO_impl *info = point;
    return info->real;
}

void STDCALL clean_codeinfo(CODEINFO *pinfo) {
    if (!pinfo || !*pinfo)
        return;
    CODEINFO_impl **info = (CODEINFO_impl **)pinfo;
    delete *info;
    *info = nullptr;
}

const char *STDCALL get_charname(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    if (info->dummyname.size()) {
        return info->dummyname.c_str();
    }
    return info->base->name.c_str();
}

const char *STDCALL get_category(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    return info->base->category.c_str();
}

unsigned int STDCALL get_ccc(CODEINFO point) {
    if (!point)
        return ~0;
    CODEINFO_impl *info = point;
    return info->base->ccc;
}

const char32_t *STDCALL get_decompsition(CODEINFO point, size_t *size) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    if (size) {
        *size = info->base->decomposition.to.size();
    }
    return info->base->decomposition.to.c_str();
}

const char *STDCALL get_decompsition_attribute(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    return info->base->decomposition.command.c_str();
}

int STDCALL is_mirrored(CODEINFO point) {
    if (!point)
        return -1;
    CODEINFO_impl *info = point;
    return info->base->mirrored;
}

const char *STDCALL get_bidiclass(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    return info->base->bidiclass.c_str();
}

const char *STDCALL get_east_asian_wides(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    return info->base->east_asian_width.c_str();
}

const char *STDCALL get_block(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    return info->base->block.c_str();
}

int STDCALL
get_numeric_digit(CODEINFO point) {
    if (!point)
        return -1;
    CODEINFO_impl *info = point;
    return info->base->numeric.v1;
}

int STDCALL get_numeric_decimal(CODEINFO point) {
    if (!point)
        return -1;
    CODEINFO_impl *info = point;
    return info->base->numeric.v2;
}

double STDCALL get_numeric_number(CODEINFO point) {
    if (!point)
        return NAN;
    CODEINFO_impl *info = point;
    return info->base->numeric.num();
}

const char *STDCALL get_numeric_number_str(CODEINFO point) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    return info->v3_str.c_str();
}

const char *STDCALL get_u8str(CODEINFO point, size_t *size) {
    if (!point)
        return nullptr;
    CODEINFO_impl *info = point;
    if (size) {
        *size = info->u8str.size();
    }
    return info->u8str.c_str();
}

template <class C>
int save_unicodedata_as_binary_impl(HUNICODEDATA data, C *filename) {
    if (!data || !filename)
        return 0;
    UnicodeData *dat = (UnicodeData *)data;
    Serializer<FileWriter> ws(filename);
    if (!ws.get().is_open())
        return 0;
    serialize_unicodedata(ws, *dat);
    return 1;
}

int STDCALL save_unicodedata_as_binary(HUNICODEDATA data, const char *filename) {
    return save_unicodedata_as_binary_impl(data, filename);
}

template <class C>
int load_text_and_save_binary_impl(C *txtfile, C *binfile) {
    if (!txtfile || !binfile) {
        return 0;
    }
    auto txt = unicodedata_from_text_impl(txtfile);
    if (!txt)
        return 0;
    auto res = save_unicodedata_as_binary_impl(txt, binfile);
    release_unicodedata(txt);
    return res;
}

int STDCALL load_text_and_save_binary(const char *txtfile, const char *binfile) {
    return load_text_and_save_binary_impl(txtfile, binfile);
}

#ifdef _WIN32
int STDCALL save_unicodedata_as_binaryW(HUNICODEDATA data, const wchar_t *filename) {
    return save_unicodedata_as_binary_impl(data, filename);
}

int STDCALL load_text_and_save_binaryW(const wchar_t *txtfile, const wchar_t *binfile) {
    return load_text_and_save_binary_impl(txtfile, binfile);
}
#endif