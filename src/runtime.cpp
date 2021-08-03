#include <argvlib.h>
#include <extutil.h>

#include <iostream>

#include "common.h"

using namespace commonlib2;

auto &Cin = cin_wrapper();
auto &Cout = cout_wrapper();
auto &Clog = cerr_wrapper();

int init_io_detail(bool sync = false, const char **err = nullptr) {
    if (!IOWrapper::Initialized()) {
        if (!IOWrapper::Init(sync, err)) {
            return 0;
        }
    }
    return 1;
}

int STDCALL init_io(int sync, const char **err) {
    return init_io_detail((bool)sync, err);
}

int STDCALL command_argv(int argc, char **argv) {
    if (!init_io_detail()) return -1;
    if (argc < 2) {
        return -1;
    }
    std::string cmd = argv[1];
    if (cmd == "txt2bin") {
        return binarymake(argc, argv);
    }
    else if (cmd == "search") {
        return search(argc, argv);
    }
    else if (cmd == "utf8" || cmd == "utf16" || cmd == "utf32") {
        return utfshow(cmd, argc, argv);
    }
    else if (cmd == "random") {
        return random_gen(argc, argv);
    }
    else if (cmd == "help") {
        const char *helpstr =
            R"(help:
    if command line argument is empty, prompt ">>>>" or ">>" will show and 
    YOU need to input more!
    help:
        show this help
    search:
        search code point info by unicodedata
        -t <file>:refer unicodedata file (txt) (default:./unicodedata.txt)
        -b <file>:refer unicodedata file (bin) (default:./unicodedata.bin)
        -u :add raw(UTF-8) charactor to infomation
        -q :reduce infomation
        -r :only raw(UTF-8) charactor with line and title
        -n :raw(UTF-8) charactor with noline and notitle (use with -r)
        -o <file>:stdout to <file>
        name <names>:
            look up code info which name property has string <names> 
        strict <names>:
            look up code info which name property has same name as <names>
        category <names>:
            look up code info which category property has string <names>
        code <numbers>:
            look up code info referred by <numbers>
        range  [<begin>]-[<end>]:
            look up code info in code range <begin> to <end>(default:0-0x10ffff)
        word <words>:
            look up code info which configure <words>
        logic <expr>:
            look up code which <expr> is true (see below)
                <expr>:=<and>|<or>|<not>|<prim>
                <and>:= "and" <expr> <expr>
                <or>:= "or" <expr> <expr>
                <not>:= "not" <expr>
                <prim>:=<name>|<strict>|<category>|<code>|<range>
                <name>:="n"<anystr>
                <strict>:="s"<anystr>
                <category>:="k"<anystr>
                <code>:="c"<number>
                <range>:="r"<number>?"-"<number>?
                <number>:=(hex(begin with "0x") or bin(begin with "0b") or digit)
    txt2bin [<option>] <txt> <bin>:
        make binary for this program from unicodedata.txt 
        <txt>:path to unicodedata.txt
        <bin>:path to unicodedata.bin (any name)
        -a <file>:refer EastAsianWide.txt
    utf8,utf16,utf32:
        convert UTF-8,UTF-16,UTF-32 for each other
        -o <file>:stdout to <file>
        -p :set hex prefix
        -n :no space (ex: e8 97 a4 -> e897a4)
        -r :show raw(UTF-8) character
        word <words>:
           convert <words> to referred UTF
        code [<option>] <codes>:
           check and convert <codes> to referred UTF
           -f <utf8|utf16|utf32>:set <codes> coecs (default:(same as command))
           -h :<codes> are hex which not begin with "0x"
           -s :<codes> are not tokenaized (ex:e897a4)
    random:
        generate random string which match conditions
        usage:random <options> <'search' command's option> <'search' subcommand>
        -c <number>:set length of string
        -d :use device(std::random_device) for generate.
        -i :show random with index
        this command wraps 'search' command and option -uqrn is unusable.
)";
        Clog << helpstr;
        return 0;
    }
    else {
        Clog << "command " << cmd << " unspported\n";
    }
    return -1;
}

template <class C>
int command_str_impl(const C *str) {
    ArgArray<char, std::string, std::vector> input;
    std::basic_string<C> in = str;
    auto tmp = split_cmd(in);
    for (auto &o : tmp) {
        if (o[0] == '"' || o[0] == '\'' || o[0] == '`') {
            o.erase(0, 1);
            o.pop_back();
        }
    }
    if (!in.size() && tmp.size()) {
        Clog << "error:invalid command input\n";
        return -1;
    }
    tmp.insert(tmp.begin(), decltype(in)());
    input.translate(std::move(tmp));
    char **argv = nullptr;
    int argc = 0;
    argv = input.argv(argc);
    return command_argv(argc, argv);
}

#if USE_CALLBACK
std::string tmpbuffer;
#endif

int STDCALL EMSCRIPTEN_KEEPALIVE command_str(const char *str, int onlybuf) {
    if (!init_io_detail()) return -1;
    if (onlybuf) {
        Cout.reset_buf();
        Cout.stop_out(true);
    }
    return command_str_impl(str);
}

int STDCALL EMSCRIPTEN_KEEPALIVE start_load() {
    return get_default_unicodedata() != nullptr;
}
#if defined(__EMSCRIPTEN__)
#if USE_CALLBACK
size_t EMSCRIPTEN_KEEPALIVE get_result_text_len() { return tmpbuffer.size(); }

const char *EMSCRIPTEN_KEEPALIVE get_result_text() {
    if (Cout.stop_out()) {
        tmpbuffer = Cout.buf_str();
        return tmpbuffer.c_str();
    }
    return "";
}

int EMSCRIPTEN_KEEPALIVE command_callback(const char *str) {
    if (command_str(str, 1) == -1) {
        return -1;
    }
    const char *c = get_result_text();
    size_t len = get_result_text_len();
    unicode_callback(c, len);
    return 0;
}
#endif
#endif

/*
#ifdef _WIN32
int command_str_wchar(const wchar_t *str) { return command_str_impl(str); }
#endif*/