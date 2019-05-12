#ifndef PF_H
#define PF_H

#include <stdlib.h>
#include <string>
#include <vector>
#include <assert.h>

namespace pf {
struct Printable
{
    struct Options {
        size_t length { 0 };
        char pad { 0 };
        enum Type {
            Text = 0,
            Integer = 'd',
            String = 's',
        } type { Text };
        std::string text;
    };

    Printable(const char *value)
        : cstr(value), srcType(CString)
    {
    }

    Printable(const std::string &value)
        : stdString(value), srcType(StdString)
    {
    }

    Printable(long long value)
        : ll(value), srcType(LongLong)
    {


    }

    ~Printable()
    {
        using namespace std;
        switch (srcType) {
        case StdString:
            stdString.~string();
        case LongLong:
        case CString:
            break;
        }
    }

    bool print(std::string &str, const Options &options)
    {
        switch (options.type) {
        case Options::Integer:
            switch (srcType) {
            case LongLong:
                str += std::to_string(ll);
                return true;
            default:
                break;
            }
            break;
        case Options::String:
            switch (srcType) {
            case CString:
                str.append(cstr);
                return true;
            case StdString:
                str.append(stdString);
                return true;
            default:
                break;
            }
        case Options::Text:
            assert(false);
            break;
        }
        return false;
    }

    union {
        std::string stdString;
        const char *cstr;
        signed long long ll;
    };

    enum SrcType {
        StdString,
        CString,
        LongLong
    } srcType;
};


template <typename T>
bool apply(std::string &ret , const std::vector<Printable::Options> &options, size_t idx)
{
    while (idx < options.size() && options[idx].type == Printable::Options::Text)
        ret += options[idx++].text;
    printf("final one %d %d\n", idx, options.size());
    return idx == options.size();
}

template <typename T>
bool apply(std::string &ret, const std::vector<Printable::Options> &options, size_t idx, const T &t)
{
    while (idx < options.size() && options[idx].type == Printable::Options::Text)
        ret += options[idx++].text;
    if (idx + 1 != options.size())
        return false;
    Printable printable(t);
    printable.print(ret, options[idx]);
    return true;
}

template <typename T, typename ... Args>
bool apply(std::string &ret, const std::vector<Printable::Options> &options, size_t idx, const T &t, const Args &...args)
{
    while (idx < options.size() && options[idx].type == Printable::Options::Text)
        ret += options[idx++].text;
    if (idx >= options.size())
        return false;
    Printable printable(t);
    printable.print(ret, options[idx++]);
    return apply(ret, options, idx, args...);
}

template <typename ... Args>
std::string format(const char *fmt, const Args &...args)
{
    std::vector<Printable::Options> opts;
    const char *last = nullptr;
    bool lastWasPercent = false;
    size_t reserve = 0;
    size_t strLen = 0;
    auto addLast = [&last, &opts, &reserve](const char *s) {
        Printable::Options opt;
        opt.text.assign(last, s - last);
        opts.push_back(opt);
        reserve += opt.text.size();
        last = nullptr;
    };
    const char *ch = fmt;
    while (*ch) {
        ++strLen;
        if (*ch == '%') {
            if (lastWasPercent) {
                assert(!last);
                if (!opts.empty() && opts.back().type == Printable::Options::Text) {
                    opts.back().text += '%';
                    ++reserve;
                } else {
                    Printable::Options opt;
                    opt.text = "%";
                    ++reserve;
                    opts.push_back(opt);
                }
            } else {
                if (last) {
                    addLast(ch);
                }
                lastWasPercent = true;
            }
        } else if (lastWasPercent) {
            if (last)
                addLast(ch);
            Printable::Options opt;
            switch (*ch) {
            case 's':
                opt.type = Printable::Options::String;
                break;
            case 'd':
                opt.type = Printable::Options::Integer;
                break;
            default:
                fprintf(stderr, "Invalid format string %%%c", *ch);
                return std::string();
            }
            opts.push_back(opt);
            lastWasPercent = false;
        } else if (!last) {
            lastWasPercent = false;
            last = ch;
        } else {
            lastWasPercent = false;
        }
        ++ch;
    }
    if (lastWasPercent) {
        fprintf(stderr, "Invalid format string %%\\0");
        return std::string();
    }
    if (last) {
        reserve += (strLen - (last - fmt));
    }
    std::string ret;
    ret.reserve(reserve);
    if (apply(ret, opts, 0, args...)) {
        if (last)
            ret.assign(last, (strLen - (last - fmt)));
        return ret;
    } else {
        fprintf(stderr, "Invalid format string\n");
        return std::string();
    }
};
}

#endif /* PF_H */
