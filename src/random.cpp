#include <array>
#include <random>

#include "common.h"

using namespace commonlib2;

template <class Engine>
int gen_randomstring(size_t count, std::u32string& seed, Engine& engine, bool index, bool noadjacent) {
    std::uniform_int_distribution<std::size_t> dist(0, seed.size() - 1);

    std::u32string result;
    size_t sum = 0;

    if (noadjacent) {
        bool ok = false;
        for (auto&& test : seed) {
            if (seed[0] != test) {
                ok = true;
                break;
            }
        }
        if (!ok) {
            Clog << "error:-l falg is true but every character is same\n";
            return -1;
        }
    }
    if (index) {
        std::u32string idxstr, countstr;
        Reader(std::to_string(seed.size() - 1)) >> idxstr;
        Reader(std::to_string(count)) >> countstr;
        result += U"count:" + countstr + U"\n";
        result += U"max index:" + idxstr + U"\n";
    }

    for (size_t i = 0; i < count; i++) {
        size_t idx = dist(engine);
        if (noadjacent) {
            if (result.size() && result.back() == seed[idx]) {
                i--;
                continue;
            }
        }
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
    if (!index) {
        Cout << "\n";
    }
#endif
    return 0;
}

void gen_random_seeds(std::vector<unsigned int>& seed_data) {
    seed_data.resize(std::mt19937_64::state_size);
    std::random_device seed_gen;
    std::generate(seed_data.begin(), seed_data.end(), std::ref(seed_gen));
}

int random_gen(int argc, char** argv) {
    int i = 2;
    size_t count = 10;
    size_t seedv = 0;
    std::vector<unsigned int> seeds;
    bool has_seed = false;
    bool device = false;
    bool index = false;
    bool noadjacent = false;
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
                else if (s == 's') {
                    std::string tmp;
                    if (!get_morearg(tmp, i, argc, argv)) {
                        return -1;
                    }
                    if (tmp == "time") {
                        seedv = time(NULL);
                        has_seed = true;
                    }
                    else if (tmp == "random") {
                        gen_random_seeds(seeds);
                        has_seed = true;
                    }
                    else {
                        if (!is_digit(tmp[0])) {
                            Clog << "warning:expect number but not\n";
                        }
                        else {
                            Reader(tmp) >> seedv;
                            has_seed = true;
                        }
                    }
                }
                else if (s == 'l') {
                    noadjacent = true;
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
        Clog << "error:need more arguments\n";
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
        return gen_randomstring(count, seed, engine, index, noadjacent);
    }
    else {
        std::mt19937_64 engine;
        if (has_seed) {
            if (seeds.size()) {
                std::seed_seq sq(seeds.begin(), seeds.end());
                engine.seed(sq);
            }
            else {
                engine.seed(seedv);
            }
        }
        return gen_randomstring(count, seed, engine, index, noadjacent);
    }
}