#include <random>

#include "common.h"

using namespace commonlib2;

template <class Engine>
int gen_randomstring(size_t count, std::u32string& seed, Engine& engine, bool index) {
    std::uniform_int_distribution<std::size_t> dist(0, seed.size() - 1);

    std::u32string result;
    size_t sum;

    if (index) {
        std::u32string idxstr, countstr;
        Reader(std::to_string(seed.size() - 1)) >> idxstr;
        Reader(std::to_string(count)) >> countstr;
        result += U"count:" + countstr + U"\n";
        result += U"max index:" + idxstr + U"\n";
    }

    for (size_t i = 0; i < count; i++) {
        size_t idx = dist(engine);
        result += seed[idx];

        if (index) {
            sum += idx;
            std::u32string idxstr;
            Reader(std::to_string(idx)) >> idxstr;
            result += U" :" + idxstr + U"\n";
        }
    }

    if (index) {
        if (count != 0) {
            Cout << "sum:" << sum << "\n";
            Cout << "average:" << (double)sum / count << "\n";
        }
    }
    std::string show;
    Reader(result) >> show;

    Cout << show;
#ifdef COMMONLIB2_IS_UNIX_LIKE
    if (!Cout.is_file() && !index) {
        Cout << "\n";
    }
#endif
    return 0;
}

int random_gen(int argc, char** argv) {
    int i = 2;
    size_t count = 10;
    bool device = false;
    bool index = false;
    bool ok = false;
    for (; i < argc; i++) {
        if (argv[i][0] == '-') {
            bool broken = false;
            for (auto&& s : std::string_view(argv[i]).substr(1)) {
                if (s == 'c') {
                    std::string tmp;
                    if (!get_morearg(tmp, i, argc, argv)) {
                        return -1;
                    }
                    constexpr size_t maxable = ~0;
                    constexpr size_t max = maxable >> 1;
                    size_t tmpnum = maxable;
                    Reader<std::string>(tmp) >> tmpnum;
                    if (tmpnum > max) {
                        Clog << "error:too large or not number:" << argv[i] << "\n";
                        return -1;
                    }
                    count = tmpnum;
                }
                else if (s == 'd') {
                    device = true;
                }
                else if (s == 'i') {
                    index = true;
                }
                else {
                    broken = true;
                    break;
                }
            }
            if (!broken) continue;
        }
        ok = true;
        break;
    }
    if (!ok) {
        Clog << "error:need more argument\n";
        return -1;
    }
    Cout.stop_out(true);
    Cout.reset_buf();
    if (search(argc, argv, i, true) == -1) {
        return -1;
    }
    std::string result = Cout.buf_str();
    Cout.reset_buf();
    Cout.stop_out(false);
    if (count && !result.size()) {
        Clog << "error:no match character exists\n";
        return false;
    }
    std::u32string seed;
    Reader(result) >> seed;
    if (device) {
        std::random_device engine;
        return gen_randomstring(count, seed, engine, index);
    }
    else {
        std::mt19937_64 engine;
        return gen_randomstring(count, seed, engine, index);
    }
}