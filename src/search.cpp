#include <unicodedata.h>

#include "common.h"

using namespace commonlib2;

bool check_charname(const std::string &str, char *cmp, bool strict) {
    return strict ? (str == cmp) : (str.find(cmp) != ~0);
}

int search(int argc, char **argv, int i, bool rnflag) {
    bool ok = false;
    bool bin = true;
    bool u8 = false;
    bool quiet = false;
    bool onlyraw = rnflag;
    bool noline = rnflag;
    std::string infile;
    bool output = false;
    for (; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] == '-') {
            for (auto c : std::string_view(arg).substr(1)) {
                if (!infile.size() && (c == 't' || c == 'b')) {
                    if (!get_morearg(infile, i, argc, argv)) {
                        return -1;
                    }
                    if (c == 'b')
                        bin = true;
                }
                else if (!u8 && c == 'u') {
                    u8 = true;
                }
                else if (!quiet && c == 'q') {
                    quiet = true;
                }
                else if (!onlyraw && c == 'r') {
                    onlyraw = true;
                }
                else if (!noline && c == 'n') {
                    noline = true;
                }
                else if (!output && c == 'o') {
                    if (!openfile(i, argc, argv)) {
                        return -1;
                    }
                    output = true;
                }
                else if (rnflag && (c == 'c' || c == 'd' || c == 'i' || c == 's' || c == 'l')) {
                }
                else {
                    Clog << "warning: ignored '" << c << "'\n";
                }
            }
            continue;
        }
        ok = true;
        break;
    }
    if (!ok) {
        Clog << "error: need more argument\n";
        return -1;
    }
    HUNICODEDATA data = nullptr;
    if (infile.size()) {
        if (bin) {
            data = unicodedata_from_binary(infile.c_str());
        }
        else {
            data = unicodedata_from_text(infile.c_str());
        }
    }
    else {
        infile = "./unicodedata.txt or ./unicodedata.bin";
        data = get_default_unicodedata();
    }
    if (!data) {
        Clog << "error:failed to load unicodedata from " << infile << "\n";
        return -1;
    }

    auto print_out = [&](CODEINFO info) {
        if (onlyraw) {
            print_u8str(info, noline);
        }
        else {
            print_codeinfo(info, u8, quiet);
        }
    };
    UnicodeData *udata = (UnicodeData *)data;
    std::string arg = argv[i];
    i++;
    if (arg == "name" || arg == "strict") {
        for (; i < argc; i++) {
            for (auto k = 0; k < 0x110000; k++) {
                CODEINFO info = nullptr;
                if (get_codeinfo(data, k, &info)) {
                    std::string str(get_charname(info));
                    if (check_charname(str, argv[i], arg[0] == 's')) {
                        print_out(info);
                    }
                    clean_codeinfo(&info);
                }
            }
        }
    }
    else if (arg == "block" || arg == "include") {
        for (; i < argc; i++) {
            for (auto k = 0; k < 0x110000; k++) {
                CODEINFO info = nullptr;
                if (get_codeinfo(data, k, &info)) {
                    std::string str(get_block(info));
                    if (check_charname(str, argv[i], arg[0] == 'i')) {
                        print_out(info);
                    }
                    clean_codeinfo(&info);
                }
            }
        }
    }
    else if (arg == "category") {
        for (; i < argc; i++) {
            for (auto k = 0; k < 0x110000; k++) {
                CODEINFO info = nullptr;
                if (get_codeinfo(data, k, &info)) {
                    std::string str(get_category(info));
                    if (str == argv[i]) {
                        print_out(info);
                    }
                    clean_codeinfo(&info);
                }
            }
        }
    }
    else if (arg == "code") {
        for (; i < argc; i++) {
            uint32_t code;
            if (!get_code(argv[i], code)) {
                continue;
            }
            CODEINFO info = nullptr;
            if (get_codeinfo(data, code, &info)) {
                print_out(info);
                clean_codeinfo(&info);
            }
            else {
                Clog << "warning: code " << argv[i]
                     << " is not unicode code point\n";
            }
        }
    }
    else if (arg == "range") {
        for (; i < argc; i++) {
            uint32_t begin, end;
            if (!get_range(argv[i], begin, end)) {
                continue;
            }
            for (uint32_t code = begin; code <= end; code++) {
                CODEINFO info = nullptr;
                if (get_codeinfo(data, code, &info)) {
                    print_out(info);
                    clean_codeinfo(&info);
                }
            }
        }
    }
    else if (arg == "logic") {
        while (i < argc) {
            Logic logic;
            if (!logic_parse(i, argc, argv, logic)) {
                release_unicodedata(data);
                return -1;
            }
            i++;
            for (auto k = 0; k < 0x110000; k++) {
                CODEINFO info = nullptr;
                if (get_codeinfo(data, k, &info)) {
                    if (logic(k, get_charname(info), get_category(info), get_block(info))) {
                        print_out(info);
                    }
                    clean_codeinfo(&info);
                }
            }
        }
    }
    else if (arg == "word") {
        for (; i < argc; i++) {
            std::u32string str;
            Reader(argv[i]) >> str;
            for (auto c : str) {
                CODEINFO info = nullptr;
                if (get_codeinfo(data, c, &info)) {
                    print_out(info);
                    clean_codeinfo(&info);
                }
                else {
                    Clog << "warning: invalid codepoint\n";
                }
            }
        }
    }
    else {
        Clog << "unsupported command:" << arg << "\n";
    }
    release_unicodedata(data);
#ifdef COMMONLIB2_IS_UNIX_LIKE
    if (!rnflag && onlyraw && noline) {
        Cout << "\n";
    }
#endif
    return 0;
}