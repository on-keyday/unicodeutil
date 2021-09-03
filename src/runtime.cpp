#include <argvlib.h>
#include <extutil.h>

#include <iostream>

#include "common.h"

using namespace commonlib2;

auto &Cin = cin_wrapper();
auto &Cout = stdout_wrapper();
auto &Clog = stderr_wrapper();

int init_io_detail(bool sync = false, const char **err = nullptr) {
    if (!IOWrapper::Initialized()) {
        if (!IOWrapper::Init(sync, err)) {
            return 0;
        }
    }
    return 1;
}

template <class C>
auto split_and_remove(std::basic_string<C> &in) {
    auto tmp = split_cmd(in);
    for (auto i = 0; i < tmp.size(); i++) {
        auto &o = tmp[i];
        if (o[0] == '"' || o[0] == '\'' || o[0] == '`') {
            o.erase(0, 1);
            o.pop_back();
        }
        if (o.size() == 0) {
            tmp.erase(tmp.begin() + i);
            i--;
        }
    }
    return tmp;
}

int STDCALL init_io(int sync, const char **err) {
    return init_io_detail((bool)sync, err);
}

int STDCALL command_argv(int argc, char **argv, int i) {
    if (!init_io_detail()) return -1;
    if (argc < 2) {
        return -1;
    }
    ArgArray arg;
    if (argv[i][0] == '-') {
        if (argv[i][1] == 'i') {
            i++;
            for (; i < argc; i++) {
                if (strcmp(argv[i], "input?") == 0) {
                    std::string input;
                    Cin.getline(input);

                    /*
                    Cout << input.size() << "\n";
                    std::wstring str;
                    Reader<std::string>(input) >> str;
                    Cout << str.size() << "\n";
                    for (auto i : str) {
                        Cout << (unsigned int)i << "\n";
                        break;
                    }
                    //*/
                    arg.push_back(std::move(input));
                }
                else {
                    arg.push_back(argv[i]);
                }
            }
        }
        else if (argv[i][1] == 'b') {
            i++;
            for (; i < argc; i++) {
                if (strcmp(argv[i], "input?") == 0) {
                    std::string input;
                    Cin.getline(input);
                    arg.translate(split_and_remove(input));
                }
                else {
                    arg.push_back(argv[i]);
                }
            }
        }
        else {
            Clog << "error:unknown option\n";
            return -1;
        }
        argv = arg.argv(argc);
        //Cout << "argc:" << argc << "\n";
        i = 0;
    }
    std::string cmd = argv[i] ? argv[i] : "";
    i++;
    if (cmd == "txt2bin") {
        return binarymake(argc, argv, i);
    }
    else if (cmd == "search") {
        return search(argc, argv, i);
    }
    else if (cmd == "utf8" || cmd == "utf16" || cmd == "utf32") {
        return utfshow(cmd, argc, argv, i);
    }
    else if (cmd == "random") {
        return random_gen(argc, argv, i);
    }
    else if (cmd == "echo") {
        for (; i < argc; i++) {
            Cout << argv[i];
        }
    }
    else if (cmd == "help") {
        const char *helpstr =
            R"(help:
    if command line argument is empty, prompt ">>>>" or ">>" will show and 
    YOU need to input more!
    -i: input from stdin (the word input? will replace stdin input)
    -b: input from stdin (the word input? will replace stdin input after split)
    help:
        show this help
    echo:
        echo cmdline
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
            look up code info which consists <words>
        block <blocknames>:
             look up code info which block property has same name as <blocknames>
        include <blocknames>:
            look up code info which block property has string <blocknames>
        logic <expr>:
            look up code which <expr> is true (see below)
                <expr>:=<and>|<or>|<not>|<prim>
                <and>:= "and" <expr> <expr>
                <or>:= "or" <expr> <expr>
                <not>:= "not" <expr>
                <prim>:=<name>|<strict>|<category>|<code>|<range>|<block>|<include>
                <name>:="n"<anystr>
                <strict>:="s"<anystr>
                <category>:="k"<anystr>
                <code>:="c"<number>
                <range>:="r"<number>?"-"<number>?
                <block>:="b"<anystr>
                <include>:="i"<anystr>
                <number>:=(hex(begin with "0x") or bin(begin with "0b") or digit)
                <anystr>:=(any string)
    txt2bin [<option>] <txt> <bin>:
        make binary for this program from unicodedata.txt 
        <txt>:path to unicodedata.txt
        <bin>:path to unicodedata.bin (any name)
        -a <file>:refer EastAsianWide.txt
        -b <fike>:refer Blocks.txt
    utf8,utf16,utf32:
        convert UTF-8,UTF-16,UTF-32 for each other
        -o <file>:stdout to <file>
        -p :set hex prefix
        -n :no space (ex: e8 97 a4 -> e897a4)
        -r :show raw(UTF-8) character
        -d :show as decimal
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
        -s <time|random|<number>>:set seed for pseudo-random number 
        -l :make sure the same characters are not adjacent
        this command wraps 'search' command and option -uqrn is unusable.
)";
        Cout << helpstr;
        return 0;
    }
    else {
        Clog << "command " << cmd << " unspported\ntype unicode help for help\n";
    }
    return -1;
}

template <class C>
int command_str_impl(const C *str) {
    ArgArray input;
    std::basic_string<C> in = str;
    auto tmp = split_and_remove(in);
    if (!in.size() && tmp.size()) {
        Clog << "error:invalid command input\n";
        return -1;
    }
    tmp.insert(tmp.begin(), decltype(in)());
    input.translate(std::move(tmp));
    char **argv = nullptr;
    int argc = 0;
    argv = input.argv(argc);
    return command_argv(argc, argv, 1);
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

int STDCALL runtime_main(int argc, char **argv) {
    const char *err = nullptr;
    if (!IOWrapper::Init(false, &err)) {
        std::cerr << err;
        return -1;
    }
    ArgChange _(argc, argv);
    /*ArgArray withpipe;
    std::string pipein;
    if (Cin.readsome(pipein) > 0) {
        withpipe.translate(argv, argv + argc);
        withpipe.translate(split_cmd(pipein));
        argv = withpipe.argv(argc);
    }*/
    if (argc < 2) {
        Cout << ">>";
        Clog << ">>";
        std::string in;
        Cin.getline(in);
        return command_str(in.c_str(), 0);
    }
    return command_argv(argc, argv, 1);
}