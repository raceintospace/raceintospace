#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>

using namespace std;

#define ARCHIVE_VECTOR(arr, type, size)                 \
    do {                                                \
        std::vector<type> vec(arr, arr+size);           \
        ar(cereal::make_nvp(#arr, vec));                \
        memcpy(arr, vec.data(), size * sizeof(type));   \
    } while (0)

#define ARCHIVE_STRING(arr)                             \
    do {                                                \
        string str = arr;                               \
        ar(cereal::make_nvp(#arr, str));                \
        strntcpy(arr, str.c_str(), sizeof(arr));        \
    } while (0)



#define SERIALIZE_XML_FILE(x, filename)         \
    do {                                        \
        ofstream os(filename);                  \
        cereal::XMLOutputArchive archive(os);   \
        archive(x);                             \
    } while (0)

#define SERIALIZE_XML_STRING(x, str)                    \
    do {                                                \
        stringstream stream;                            \
        {                                               \
            cereal::XMLOutputArchive archive(stream);   \
            archive(x);                                 \
        }                                               \
        str = stream.str();                             \
    } while (0)

#define DESERIALIZE_XML_FILE(x, filename)                \
    do {                                                 \
        ifstream is(filename);                           \
        cereal::XMLInputArchive iarchive(is);            \
        iarchive(*x);                                    \
    } while (0)

#define SERIALIZE_JSON_FILE(x, filename)                                \
    do {                                                                \
        ofstream os(filename);                                          \
        cereal::JSONOutputArchive archive(os);                          \
        archive(x);                                                     \
    } while (0)

#define DESERIALIZE_JSON_FILE(x, filename)       \
    do {                                         \
        ifstream is(filename);                   \
        cereal::JSONInputArchive iarchive(is);   \
        iarchive(*x);                            \
    } while (0)


/* Null-terminated version of strncpy */
static inline char *strntcpy(char *dest, const char *src, size_t n)
{
    strncpy(dest, src, n);
    dest[n - 1] = 0;
    return dest;
}

#endif //SERIALIZE_H
