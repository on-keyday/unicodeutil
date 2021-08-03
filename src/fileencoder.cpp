#include <fileio.h>

#include "common.h"
using namespace commonlib2;

bool read_as_utf8bom(Reader<FileReader>& r) {
    Reader<ToUTF32<FileReader>> u8(std::move(r.ref()));
    if (u8.ref().size() == 0) {
        Clog << "warning:expected UTF-8 BOM but not decodable\n";
        r.ref() = std::move(u8.ref().base_reader().ref());
        return false;
    }
    u8.increment();
}

template <class T>
bool read_as_utfwithbom(Reader<FileReader>& r, bool be, const char* utf) {
    Reader<Order<FileReader, T>> u(std::move(r.ref()));
    if (u.ref().size() == 0) {
        Clog << "warning:expected"
             << utf
             << "BOM(" << (be ? "BE" : "LE")
             << ") but not decodable\n";
        r.ref() = std::move(u.ref().ref());
        return false;
    }
    if CONSTEXPRIF (is_big_endian()) {
        if (be) u.ref().set_reverse(true);
    }
    else {
        if (!be) u.ref().set_reverse(true);
    }
    if CONSTEXPRIF (sizeof(T)) {
        u.increment();
    }
    else {
    }
}

int file_encoder(int argc, char** argv) {
    int i = 2;
    bool ok = false;
    std::string expects;
    std::string input, output;
    for (; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (auto&& c : std::string_view(argv[i]).substr(1)) {
                if (!expects.size() && c == 'e') {
                    if (!get_morearg(expects, i, argc, argv)) {
                        return -1;
                    }
                }
                else {
                    Clog << "warning: ignored '" << c << "'\n";
                }
            }
            continue;
        }
        else if (!input.size()) {
            input = argv[i];
            continue;
        }
        else if (!output.size()) {
            output = argv[i];
            continue;
        }
        break;
    }
#ifdef _WIN32
    std::wstring tmp;
#else
    std::string tmp;
#endif
    Reader(input) >> tmp;
    Reader<FileReader> r(tmp.c_str());
    if (!r.ref().is_open()) {
        Clog << "error:file " << input << " couldn't open\n";
        return -1;
    }
    if (r.ahead("\xEF\xBB\xBF")) {
        if (read_as_utf8bom(r)) {
            return 0;
        }
    }
    else if (r.ahead(Sized("\x00\x00\xfe\xff"))) {
    }
    return 0;
}