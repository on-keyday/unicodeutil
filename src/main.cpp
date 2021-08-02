#include <argvlib.h>

#include <iostream>

#include "runtime.h"

using namespace commonlib2;

int main(int argc, char **argv) {
    const char *err = nullptr;
    if (!IOWrapper::Init(false, &err)) {
        std::cerr << err;
        return -1;
    }
    ArgChange _(argc, argv);
    if (argc < 2) {
        Cout << ">>";
        Clog << ">>";
        std::string in;
        Cin.getline(in);
        return command_str(in.c_str(), 0);
    }
    return command_argv(argc, argv);
}