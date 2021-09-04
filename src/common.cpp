#include "common.h"

using namespace commonlib2;

bool get_morearg(std::string &output, int &i, int argc, char **argv) {
    i++;
    if (i >= argc) {
        Clog << "error:need file path\n";
        return false;
    }
    output = argv[i];
    return true;
}

bool openfile(int &i, int argc, char **argv) {
    std::string output;
    if (!get_morearg(output, i, argc, argv)) {
        return false;
    }
    if (!Cout.open(output)) {
        Clog << "error:out put file " << output << " not opened";
        return false;
    }
    return true;
}

void print_u8str(CODEINFO info, bool noline, const char *prefix) {
    size_t size = 0;
    const char *str = get_u8str(info, &size);
    if (noline) {
        Cout << std::string(str, size);
    }
    else {
        Cout << prefix << std::string(str, size) << "\n";
    }
}

void print_codeinfo(CODEINFO info, bool u8, bool few) {
    Cout << "------------------";
    Cout << "point: 0x" << std::hex << (unsigned int)get_codepoint(info)
         << "\n";
    Cout << "name: " << get_charname(info) << "\n";
    if (!few) {
        if (auto block = get_block(info); block[0]) {
            Cout << "block: " << block << "\n";
        }
        Cout << "category: " << get_category(info) << "\n";
        Cout << "ccc: " << get_ccc(info) << "\n";
        Cout << "bidiclass: " << get_bidiclass(info) << "\n";
        Cout << "east_asian_width: " << get_east_asian_wides(info) << "\n";
        Cout << "mirrored: " << std::boolalpha << (bool)is_mirrored(info)
             << "\n";
    }
    if (u8) {
        print_u8str(info, false);
    }
    if (!few) {
        size_t i = 0;
        const char32_t *p = get_decompsition(info, &i);
        if (i) {
            Cout << "decomposition: ";
            for (auto s = 0; s < i; s++) {
                if (s) {
                    Cout << " ";
                }
                Cout << "0x" << std::hex << (uint32_t)p[s];
            }
            if (u8) {
                std::string raw;
                Reader(std::u32string(p, i)) >> raw;
                Cout << " <raw:" << raw << ">";
            }
            Cout << "\n";
            if (auto i = get_decompsition_attribute(info); i[0] != 0)
                Cout << "decomposition ext: "
                     << get_decompsition_attribute(info) << "\n";
        }
        if (auto i = get_numeric_digit(info); i != -1)
            Cout << "digit: " << i << "\n";
        if (auto i = get_numeric_decimal(info); i != -1)
            Cout << "decimal: " << i << "\n";
        if (auto i = get_numeric_number_str(info); i[0] != 0)
            Cout << "number: " << i << "\n";
    }
}

bool get_range(const char *str, uint32_t &begin, uint32_t &end, const char *msg) {
    begin = ~0, end = ~0;
    Reader f(str);
    if (f.expect("-")) {
        begin = 0;
    }
    else {
        f >> begin;
        if (!f.expect("-")) {
            Clog << msg
                 << ": syntax of range is invalid: <code begin>-<code end> "
                    "[begin:end]\n";
            return false;
        }
    }
    if (f.ceof()) {
        end = 0x10ffff;
    }
    else {
        f >> end;
    }
    if (begin == ~0 || end == ~0 || end > 0x10ffff) {
        Clog << msg
             << ": syntax of range is invalid: <code begin>-<code end> "
                "[begin:end]\n";
        return false;
    }
    return true;
}

bool get_code(const char *str, uint32_t &code, const char *msg) {
    code = ~0;
    Reader(str) >> code;
    if (code == ~0 || code > 0x10ffff) {
        Clog << msg << ": " << str << " is not parseable as number\n";
        return false;
    }
    return true;
}

bool logic_parse(int &i, int argc, char **argv, Logic &logic) {
    bool ok = false;
    if (i < argc) {
        std::string arg = argv[i];
        if (arg == "and" || arg == "or") {
            i++;
            Logic left, right;
            if (!logic_parse(i, argc, argv, left)) {
                return false;
            }
            i++;
            if (!logic_parse(i, argc, argv, right)) {
                return false;
            }
            logic.type = arg == "and" ? LogicType::and_ : LogicType::or_;
            logic.child.push_back(std::move(left));
            logic.child.push_back(std::move(right));
        }
        else if (arg == "not") {
            i++;
            Logic op;
            if (!logic_parse(i, argc, argv, op)) {
                return false;
            }
            logic.type = LogicType::not_;
            logic.child.push_back(std::move(op));
        }
        else if (arg[0] == 'r') {
            arg.erase(0, 1);
            if (!get_range(arg.c_str(), logic.code1, logic.code2, "error")) {
                return false;
            }
            logic.type = LogicType::range;
        }
        else if (arg[0] == 'n' || arg[0] == 's' || arg[0] == 'k' || arg[0] == 'b' || arg[0] == 'i') {
            logic.type = arg[0] == 's'   ? LogicType::strict
                         : arg[0] == 'k' ? LogicType::category
                         : arg[0] == 'b' ? LogicType::block
                         : arg[0] == 'i' ? LogicType::includes
                                         : LogicType::name;
            arg.erase(0, 1);
            logic.str = std::move(arg);
        }
        else if (arg[0] == 'c') {
            arg.erase(0, 1);
            if (!get_code(arg.c_str(), logic.code1, "error")) {
                return false;
            }
            logic.type = LogicType::code;
        }
        else {
            Clog << "error:invalid prefix: allows: r-range c-code n-name "
                    "s-strict b-block not and or\n";
            return false;
        }
        ok = true;
    }
    if (!ok) {
        Clog << "error:invalid logic\n";
        return false;
    }
    return true;
}