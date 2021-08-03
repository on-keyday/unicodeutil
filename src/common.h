#pragma once
#include <coutwrapper.h>
#include <extension_operator.h>

#include <vector>
#define DLL_EXPORT __declspec(dllexport)
#include "runtime.h"
#include "unicodeload.h"

enum class LogicType {
    and_,
    or_,
    not_,
    range,
    strict,
    name,
    code,
    category,
};

struct Logic {
    LogicType type;
    std::string str;
    uint32_t code1 = ~0;
    uint32_t code2 = ~0;
    std::vector<Logic> child;

    bool operator()(uint32_t code, const char *name, const char *category) {
        switch (type) {
            case LogicType::and_:
                return child[0](code, name, category) && child[1](code, name, category);
            case LogicType::or_:
                return child[0](code, name, category) || child[1](code, name, category);
            case LogicType::not_:
                return !child[0](code, name, category);
            case LogicType::range:
                return code >= code1 && code <= code2;
            case LogicType::strict:
                return str == name;
            case LogicType::name:
                return std::string(name).find(str) != ~0;
            case LogicType::code:
                return code1 == code;
            case LogicType::category:
                return std::string(category).find(str) != ~0;
        }
    }
};

void print_u8str(CODEINFO info, bool noline, const char *prefix = "raw: ");
void print_codeinfo(CODEINFO info, bool u8, bool few);
bool get_code(const char *str, uint32_t &code, const char *msg = "warning");
bool logic_parse(int &i, int argc, char **argv, Logic &logic);
bool get_range(const char *str, uint32_t &begin, uint32_t &end, const char *msg = "warning");

int binarymake(int argc, char **argv);

bool get_morearg(std::string &output, int &i, int argc, char **argv);

bool openfile(int &i, int argc, char **argv);

int search(int argc, char **argv, int i = 2, bool rnflag = false);

int utfshow(std::string &cmd, int argc, char **argv);

int random_gen(int argc, char **argv);