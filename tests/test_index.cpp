#include "gtest.h"
#include "indexer/index.h"
#include <vector>
#include <algorithm>


using namespace std;


using Byte = unsigned char;


template<typename ContainerA, typename ContainerB>
bool setEquals(ContainerA a, ContainerB b) {
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    return equal(a.begin(), a.end(), b.begin(), b.end());
}


template<typename ContainerA, typename ContainerB>
bool setIncludes(ContainerA a, ContainerB b) { // a is subset of b
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    return includes(b.begin(), b.end(), a.begin(), a.end());
}


void testByStrings(const string &query, vector<string> const &expected, Index &index){
    EXPECT_TRUE(setEquals(expected, index.findGoodNames(query)));
}


template <typename CharGenerator>
string getRandomString(int size, CharGenerator generator){
    string result;
    for (int i = 0; i < size; i++) {
        result.push_back(generator());
    }
    return result;
}


string getRandomString(int size) {
    static auto defaultCharGenerator = [](){return rand() % 26 + 'a';};
    return getRandomString(size, defaultCharGenerator);
}


TEST(correctness, simple_test) {
    stringstream data1("a bcd");
    stringstream data2("bcde");

    Index index;
    index.add("data1", data1);
    index.add("data2", data2);

    testByStrings(" bc", vector<string>({"data1"}), index);
    testByStrings("bcd", vector<string>({"data1", "data2"}), index);
    testByStrings("cde", vector<string>({"data2"}), index);
    testByStrings("a bcd", vector<string>({"data1"}), index);
}


TEST(correctness, less_than_three_test) {
    stringstream data1("a");
    stringstream data2("abac");

    Index index;
    index.add("data1", data1);
    index.add("data2", data2);

    testByStrings("a", vector<string>({"data1", "data2"}), index);
    testByStrings("b", vector<string>({"data1", "data2"}), index);
    testByStrings("bac", vector<string>({"data2"}), index);
}


TEST(correctness, random_test) {
    const int INDEX_SIZE = 1000;
    const int QUERIES_COUNT = 10;

    set<string> content;
    Index index;
    for (int i = 0; i < INDEX_SIZE; i++) {

        if (i % 1000 == 0) cerr << "inserted: " << i << endl;

        int size = rand() % 1000 + 1;
        string str = getRandomString(size);
        if (!content.insert(str).second) {
            i--;
            continue;
        }
        stringstream stream(str);
        index.add(str, stream);
    }

    for (int i = 0; i < QUERIES_COUNT; i++) {
        int size = rand() % 10;
        std::vector<std::string> ans;
        string query = getRandomString(size);
        for (auto const &str : content){
            if (str.find(query) != std::string::npos) {
                ans.push_back(str);
            }
        }
        auto res = index.findGoodNames(query);
        bool isIncludes = setIncludes(ans, res);
        EXPECT_TRUE(isIncludes);
    }
}


TEST(correctness, remove_test) {
    stringstream data1("a");
    stringstream data2("abac");

    Index index;
    index.add("data1", data1);
    index.add("data2", data2);

    testByStrings("a", vector<string>({"data1", "data2"}), index);
    index.remove("data1");
    testByStrings("a", vector<string>({"data2"}), index);
    index.remove("data2");
    testByStrings("a", vector<string>(), index);
}