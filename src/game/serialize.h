#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <algorithm>
#include <cstream>
#include <fstream>

#include <cereal/cereal.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>

#define ARCHIVE_VECTOR(arr, type, size)                 \
    do {                                                \
        std::vector<type> vec(arr, arr+size);           \
        ar(cereal::make_nvp(#arr, vec));                \
        memcpy(arr, vec.data(), size * sizeof(type));   \
    } while (0)

#define ARCHIVE_STRING(arr)                             \
    do {                                                \
        std::string str = arr;                          \
        ar(cereal::make_nvp(#arr, str));                \
        strntcpy(arr, str.c_str(), sizeof(arr));        \
    } while (0)

#define ARCHIVE_ARRAY(arr, type)                                   \
    do {                                                           \
        std::vector<type> vec(arr.begin(), arr.end());             \
        ar(cereal::make_nvp(#arr, vec));                           \
        std::copy_n(vec.begin(), arr.size(), arr.begin());         \
    } while (0)

#define SERIALIZE_XML_FILE(x, filename)         \
    do {                                        \
        std::ofstream os{filename};             \
        cereal::XMLOutputArchive archive{os};   \
        archive(x);                             \
    } while (0)

#define SERIALIZE_XML_STRING(x, str)                    \
    do {                                                \
        std::stringstream stream;                       \
        {                                               \
            cereal::XMLOutputArchive archive{stream};   \
            archive(x);                                 \
        }                                               \
        str = stream.str();                             \
    } while (0)

#define DESERIALIZE_XML_FILE(x, filename)                \
    do {                                                 \
        std::ifstream is{filename};                      \
        cereal::XMLInputArchive iarchive{is};            \
        iarchive(*x);                                    \
    } while (0)

#define SERIALIZE_JSON_FILE(x, filename)                                \
    do {                                                                \
        std::ofstream os{filename};                                     \
        cereal::JSONOutputArchive archive{os};                          \
        archive(x);                                                     \
    } while (0)

#define DESERIALIZE_JSON_FILE(x, filename)       \
    do {                                         \
        std::ifstream is{filename};              \
        cereal::JSONInputArchive iarchive{is};   \
        iarchive(*x);                            \
    } while (0)

#define ASSERT(expr)                                                    \
    do {                                                                \
        if (!(expr)) {                                                  \
            std::stringstream err;                                      \
            err << "Sanitization check failed: " <<  #expr << " (" << __FILE__ << ":" << __LINE__ << ")"; \
            throw std::out_of_range(err.str());                         \
        }                                                               \
    } while (0)


/* Null-terminated version of strncpy */
static inline char* strntcpy(char* dest, const char* src, size_t n)
{
    if (n == 0) return dest;
    strncpy(dest, src, n-1);
    dest[n - 1] = 0;
    return dest;
}

#endif //SERIALIZE_H
