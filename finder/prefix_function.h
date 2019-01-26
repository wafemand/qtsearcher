#pragma once


#include <vector>


template<class T>
class prefix_function {


private:
    std::vector<size_t> pref_func;
    T str;
    size_t common_size = 0;


    template<typename Tchar>
    size_t calc_next_pf(Tchar new_char) {
        size_t ret = common_size;
        while (ret > 0 && new_char != str[ret]) {
            ret = pref_func[ret - 1];
        }
        if (new_char == str[ret]) {
            ret++;
        }
        return ret;
    }


    void build_pf() {
        common_size = 0;
        size_t n = str.size();
        pref_func.resize(n);
        pref_func[0] = 0;
        for (size_t i = 1; i < n; i++) {
            pref_func[i] = calc_next_pf(str[i]);
        }
    }


public:
    explicit prefix_function() = default;


    explicit prefix_function(T const &st) {
        str = st;
        build_pf();
    }


    bool matched() {
        return common_size == str.size();
    }


    template<typename Tchar>
    void update(Tchar new_char) {
        common_size = calc_next_pf(new_char);
    }


    void rebuild(T const &new_string) {
        str = new_string;
        build_pf();
    }
};
