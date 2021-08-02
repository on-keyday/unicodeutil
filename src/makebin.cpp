#include <unicodedata.h>

#include "common.h"

using namespace commonlib2;
int binarymake(int argc, char **argv) {
    std::string asianfile, txtfile, binfile;
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (!asianfile.size() && arg == "-a") {
            i++;
            if (i >= argc) {
                Clog << "error:need file path\n";
                return -1;
            }
            asianfile = argv[i];
        }
        else if (!txtfile.size()) {
            txtfile = std::move(arg);
        }
        else if (!binfile.size()) {
            binfile = std::move(arg);
        }
        else {
            break;
        }
    }

    if (!txtfile.size() && !binfile.size()) {
        Clog << "need txt file name and binary file name\n";
        return -1;
    }
#ifdef _WIN32
    std::wstring tmp;
    Reader(txtfile) >> tmp;
    HUNICODEDATA data = unicodedata_from_textW(tmp.c_str());
#else
    HUNICODEDATA data = unicodedata_from_text(txtfile.c_str());
#endif
    if (!data) {
        Clog << "error:failed to load unicodedata from " << txtfile << "\n";
        return -1;
    }
    if (asianfile.size()) {
#ifdef _WIN32
        tmp = L"";
        Reader(asianfile) >> tmp;
        auto vec = load_EastAsianWide_text(tmp.c_str());
#else
        auto vec = load_EastAsianWide_text(asianfile.c_str());
#endif
        if (!vec.size()) {
            Clog << "error:failed to load east asian wide from " << asianfile
                 << "\n";
            return -1;
        }
        if (!apply_east_asian_wide(vec, *(UnicodeData *)data)) {
            Clog << "error:failed to apply east asian wide from " << asianfile
                 << "\n";
            return -1;
        }
    }
#if _WIN32
    tmp = L"";
    Reader(binfile) >> tmp;
    if (!save_unicodedata_as_binaryW(data, tmp.c_str())) {
#else
    if (!save_unicodedata_as_binary(data, binfile.c_str())) {
#endif
        Clog << "error:failed to write unicodedata to " << binfile << "\n";
    }
    release_unicodedata(data);
    Clog << "operation succeed: unicodedata was written to " << binfile << "\n";
    return 0;
}