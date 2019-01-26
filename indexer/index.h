#pragma once


#include <unordered_map>
#include <set>
#include <sstream>
#include <map>
#include <algorithm>


struct CancelException : std::exception {};


class Index {
    const size_t BINARY_SIZE = 200000;

    using Trigram = uint32_t;
    using Symbol = unsigned char;

public:
    Index() = default;


    template<typename InputStream>
    void add(std::string const &name, InputStream &stream) {
        TrigramIterator<InputStream> it(stream);
        addTrigrams(name, it);
    }


    void remove(std::string const &name) {
        nameToTrigrams.erase(name);
    }


    size_t size() {
        return nameToTrigrams.size();
    }


    template <typename Callback>
    void forEachGoodName(std::string const &query, Callback callback) {
        if (query.size() < 3) {
            for (const auto &kv : nameToTrigrams) {
                callback(kv.first);
            }
            return;
        }

        std::stringstream queryStream(query);
        TrigramIterator<std::stringstream> trigramIterator(queryStream);
        std::vector<Trigram> queryTrigrams;
        while (trigramIterator.hasNext()) {
            queryTrigrams.push_back(trigramIterator.next());
        }
        std::sort(queryTrigrams.begin(), queryTrigrams.end());

        for (auto const &kv : nameToTrigrams) {
            auto &trigrams = kv.second;
            auto it = trigrams.begin();
            bool allFound = true;
            for (Trigram trigram : queryTrigrams) {
                it = std::lower_bound(it, trigrams.end(), trigram);
                if (it == trigrams.end() || *it != trigram) {
                    allFound = false;
                    break;
                }
            }
            cancellation_point();
            if (allFound) {
                callback(kv.first);
            }
        }
    }


    std::vector<std::string> findGoodNames(std::string const &query) {
        std::vector<std::string> result;

        forEachGoodName(query, [&result](std::string name) {
            result.push_back(name);
        });

        return result;
    }


    template<typename InputStream>
    void load(std::string const &name, InputStream &stream) {
        std::vector<Trigram> trigrams;
        while (stream) {
            Symbol c1 = stream.get();
            Symbol c2 = stream.get();
            Symbol c3 = stream.get();
            if (stream) {
                trigrams.push_back(getTrigram(c1, c2, c3));
            }
        }

        std::sort(trigrams.begin(), trigrams.end());
        nameToTrigrams[name] = std::move(trigrams);
    }


    template<typename OutputStream>
    void save(std::string const &name, OutputStream &stream) {
        for (Trigram trigram : nameToTrigrams[name]){
            auto str = trigramToString(trigram);
            stream << str;
        }
    }


    std::vector<std::string> getNames() {
        std::vector<std::string> result;
        result.reserve(nameToTrigrams.size());
        for (auto const &kv : nameToTrigrams){
            result.push_back(kv.first);
        }
        return result;
    }


    size_t infoSize(std::string const &name) {
        return nameToTrigrams[name].size() * 3 * sizeof(Symbol);
    }


    void cancel() {
        cancelled = true;
    }

private:
    void cancellation_point() {
        if (cancelled) {
            cancelled = false;
            throw CancelException();
        }
    }


    template <typename Iterator>
    void addTrigrams(std::string const &name, Iterator &it) {
        std::set<Trigram> trigrams;
        while (it.hasNext()) {
            Trigram cur = it.next();
            trigrams.insert(cur);
            if (trigrams.size() > BINARY_SIZE) {
                remove(name);
                return;
            }
        }
        std::copy(trigrams.begin(), trigrams.end(), std::back_inserter(nameToTrigrams[name]));
    }


    template<typename InputStream>
    class TrigramIterator {
    public:
        explicit TrigramIterator(InputStream &stream) : stream_(std::move(stream)) {
            s0 = stream_.get();
            if (!stream_) {
                return;
            }
            s1 = stream_.get();
            if (!stream_) {
                return;
            }
            s2 = stream_.get();
        }

        Trigram next() {
            Symbol prev0 = s0;
            s0 = s1;
            s1 = s2;
            s2 = stream_.get();
            return getTrigram(prev0, s0, s1);
        }

        bool hasNext() {
            return bool(stream_);
        }

    private:
        InputStream stream_;
        Symbol s0 = 0, s1 = 0, s2 = 0;
    };


    static std::string trigramToString(Trigram trigram) {
        const uint p = 1 << (sizeof(Symbol) * 8);
        char c1 = char(trigram % p);
        char c2 = char(trigram / p % p);
        char c3 = char(trigram / p / p);
        auto ret = std::string({c1, c2, c3});
        return ret;
    }


    static Trigram getTrigram(std::string const &str) {
        assert(str.size() == 3);
        return getTrigram(str[0], str[1], str[2]);
    }


    static Trigram getTrigram(Symbol s0, Symbol s1, Symbol s2) {
        const uint p = 1 << (sizeof(Symbol) * 8);
        return s0 +
               s1 * p +
               s2 * p * p;
    }


    std::map<std::string, std::vector<Trigram> > nameToTrigrams;
    bool cancelled = false;
};