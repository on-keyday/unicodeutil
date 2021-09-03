#include <argvlib.h>
#include <extutil.h>

#include <iomanip>

#include "common.h"

using namespace commonlib2;

struct FormatFlags {
    bool prefix = false;
    bool space = true;
    bool raw = false;
    bool decimal = false;
};

template <class C>
void print_as_command(const C& str, std::string& cmd, FormatFlags& flags) {
    bool first = true;
    auto print_prefix = [&] {
        if (flags.space && !first) {
            Cout << " ";
        }
        if (flags.prefix) {
            Cout << "0x";
        }
    };
    if (cmd == "utf8") {
        std::string tmp;
        Reader(str) >> tmp;
        for (auto& c : tmp) {
            print_prefix();
            if (!flags.decimal) {
                Cout << std::setfill('0') << std::hex << std::setw(2);
            }
            Cout << (unsigned int)(unsigned char)c;
            first = false;
        }
    }
    else if (cmd == "utf16") {
        std::u16string tmp;
        Reader(str) >> tmp;
        for (auto& c : tmp) {
            print_prefix();
            if (!flags.decimal) {
                Cout << std::setfill('0') << std::hex << std::setw(4);
            }
            Cout << (unsigned int)c;
            first = false;
        }
    }
    else if (cmd == "utf32") {
        std::u32string tmp;
        Reader(str) >> tmp;
        for (auto& c : tmp) {
            print_prefix();
            if (!flags.decimal) {
                Cout << std::setfill('0') << std::hex << std::setw(8);
            }
            Cout << (unsigned int)c;
            first = false;
        }
    }
}

void print_line() {
#ifdef COMMONLIB2_IS_UNIX_LIKE
    Cout << "\n";
#endif
}

void outputword(std::string& cmd, int& i, int argc, char** argv, FormatFlags& flags) {
    for (; i < argc; i++) {
        if (flags.raw) {
            Cout << argv[i];
        }
        else {
            print_as_command(argv[i], cmd, flags);
        }
    }
    print_line();
}

bool change_nospacenumber(ArgArray<>& arg, std::string& from, int& i,
                          int& argc, char**& argv) {
    std::string nospacenumber, result;
    for (; i < argc; i++) {
        nospacenumber += argv[i];
    }
    int sp = 0;
    bool res = false;
    auto check_mod = [&](unsigned int m) {
        if (nospacenumber.size() % m != 0) {
            Clog << "error:for" << from << ", nospace number's length is not mod " << m << "\n";
            res = false;
        }
        else {
            sp = m;
            res = true;
        }
    };

    if (from == "utf8") {
        check_mod(2);
    }
    else if (from == "utf16") {
        check_mod(4);
    }
    else if (from == "utf32") {
        check_mod(8);
    }
    else {
        Clog << "error:invalid UTF name:" << from << "\n";
        return false;
    }
    if (!res) return false;
    for (size_t i = 0; i < nospacenumber.size(); i++) {
        if (i != 0 && i % sp == 0) {
            result += " ";
        }
        result += nospacenumber[i];
    }
    arg.translate(split(result, " "));
    argv = arg.argv(argc);
    i = 0;
    return true;
}

int show_code(std::string& cmd, int& i, int argc, char** argv, FormatFlags& flags) {
    std::string from;
    bool ok = false;
    bool must16 = false;
    bool nospace = false;
    for (; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (auto& c : std::string_view(argv[i]).substr(1)) {
                if (!from.size() && c == 'f') {
                    if (!get_morearg(from, i, argc, argv)) {
                        return -1;
                    }
                }
                else if (!must16 && c == 'h') {
                    must16 = true;
                }
                else if (!nospace && c == 's') {
                    nospace = true;
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
        Clog << "error:need more arguments\n";
        //<< argc << ":" << i;
        return -1;
    }
    if (!from.size()) {
        from = cmd;
    }
    ArgArray arg;
    if (nospace) {
        if (!change_nospacenumber(arg, from, i, argc, argv)) {
            return false;
        }
        must16 = true;
    }
    std::u32string converted;
    int err = 0;
    if (from == "utf8") {
        std::string u8str;
        for (; i < argc; i++) {
            unsigned short code = ~0;
            std::string s;
            if (must16) {
                s = "0x";
            }
            s += argv[i];
            Reader(s) >> code;
            if (code > 0xff) {
                Clog << "error:too large or not number: " << argv[i] << "\n";
                return -1;
            }
            u8str.push_back((unsigned char)code);
        }
        Reader(std::move(u8str)).readwhile(converted, utf8toutf32, &err);
        if (err) {
            Clog << "error: invalid utf8 codes. error at: " << err << "\n";
            return -1;
        }
    }
    else if (from == "utf16") {
        std::u16string u16str;
        for (; i < argc; i++) {
            unsigned int code = ~0;
            std::string s;
            if (must16) {
                s = "0x";
            }
            s += argv[i];
            Reader(s) >> code;
            if (code > 0xffff) {
                Clog << "error:too large or not number: " << argv[i] << "\n";
                return -1;
            }
            u16str.push_back(code);
        }
        Reader(std::move(u16str)).readwhile(converted, utf16toutf32, &err);
        if (err) {
            Clog << "error: invalid utf16 codes. error at: " << err << "\n";
            return -1;
        }
    }
    else if (from == "utf32") {
        for (; i < argc; i++) {
            unsigned int code = ~0;
            std::string s;
            if (must16) {
                s = "0x";
            }
            s += argv[i];
            Reader(s) >> code;
            if (code > 0x10ffff) {
                Clog << "error:too large or not number: " << argv[i] << "\n";
                return -1;
            }
            converted.push_back(code);
        }
    }
    else {
        Clog << "error:invalid UTF name:" << from << "\n";
        return -1;
    }
    if (flags.raw) {
        std::string tmp;
        Reader(converted) >> tmp;
        Cout << tmp;
    }
    else {
        print_as_command(converted, cmd, flags);
    }
    print_line();
    return 0;
}

int show_range(std::string& cmd, int& i, int argc, char** argv, FormatFlags& flags) {
    std::u32string converted;
    for (; i < argc; i++) {
        uint32_t begin, end;
        if (get_range(argv[i], begin, end)) {
            for (auto k = begin; k <= end; k++) {
                converted.push_back(k);
            }
        }
    }

    if (flags.raw) {
        std::string tmp;
        Reader(converted) >> tmp;
        Cout << tmp;
    }
    else {
        print_as_command(converted, cmd, flags);
    }
    print_line();
    return 0;
}

int utfshow(std::string& cmd, int argc, char** argv, int i) {
    bool ok = false;
    bool output = false;
    FormatFlags flags;
    for (; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (auto& c : std::string_view(argv[i]).substr(1)) {
                if (!output && c == 'o') {
                    if (!openfile(i, argc, argv)) {
                        return -1;
                    }
                    output = true;
                }
                else if (!flags.prefix && c == 'p') {
                    flags.prefix = true;
                }
                else if (flags.space && c == 'n') {
                    flags.space = false;
                }
                else if (!flags.raw && c == 'r') {
                    flags.raw = true;
                }
                else if (!flags.decimal && c == 'd') {
                    flags.decimal = true;
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
        Clog << "error:need more arguments\n";
        return -1;
    }
    std::string arg = argv[i];
    i++;
    if (arg == "word") {
        outputword(cmd, i, argc, argv, flags);
    }
    else if (arg == "code") {
        return show_code(cmd, i, argc, argv, flags);
    }
    else if (arg == "range") {
        return show_range(cmd, i, argc, argv, flags);
    }
    else {
        Clog << "unsurpported command: " << arg;
    }
    return 0;
}