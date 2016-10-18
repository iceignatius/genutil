/**
 * @file
 * @brief     URL parser.
 * @details   Parse URL string.
 * @author    王文佑
 * @date      2015.04.16
 * @copyright ZLib Licence
 * @see       http://www.openfoundry.org/of/projects/2419
 */
#ifndef _GEN_NET_URLPAR_H_
#define _GEN_NET_URLPAR_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#include <string>
#endif

#ifdef __cplusplus
extern "C" {
#endif

char* urlpar_extract_sceme (char *dest, size_t destsz, const char *url);
char* urlpar_extract_user  (char *dest, size_t destsz, const char *url);
char* urlpar_extract_pass  (char *dest, size_t destsz, const char *url);
char* urlpar_extract_host  (char *dest, size_t destsz, const char *url);
char* urlpar_extract_port  (char *dest, size_t destsz, const char *url);
char* urlpar_extract_path  (char *dest, size_t destsz, const char *url);
char* urlpar_extract_query (char *dest, size_t destsz, const char *url);
char* urlpar_extract_fragid(char *dest, size_t destsz, const char *url);

uint16_t urlpar_get_port(const char *url);

#ifdef __cplusplus
}  // extern "C"
#endif

#ifdef __cplusplus

/// C++ wrapper of URL parser.
namespace UrlPar
{

inline std::string ExtractSceme(const std::string &URL)
{
    /// @see ::urlpar_extract_sceme
    char Result[URL.length()+1];
    return urlpar_extract_sceme(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractUser(const std::string &URL)
{
    /// @see ::urlpar_extract_user
    char Result[URL.length()+1];
    return urlpar_extract_user(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractPass(const std::string &URL)
{
    /// @see ::urlpar_extract_pass
    char Result[URL.length()+1];
    return urlpar_extract_pass(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractHost(const std::string &URL)
{
    /// @see ::urlpar_extract_host
    char Result[URL.length()+1];
    return urlpar_extract_host(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractPort(const std::string &URL)
{
    /// @see ::urlpar_extract_port
    char Result[URL.length()+1];
    return urlpar_extract_port(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractPath(const std::string &URL)
{
    /// @see ::urlpar_extract_path
    char Result[URL.length()+1];
    return urlpar_extract_path(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractQuery(const std::string &URL)
{
    /// @see ::urlpar_extract_query
    char Result[URL.length()+1];
    return urlpar_extract_query(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline std::string ExtractFragID(const std::string &URL)
{
    /// @see ::urlpar_extract_fragid
    char Result[URL.length()+1];
    return urlpar_extract_fragid(Result, sizeof(Result), URL.c_str()) ?
           Result : "";
}

inline uint16_t GetPort(const std::string &URL)
{
    /// @see ::urlpar_get_port
    return urlpar_get_port(URL.c_str());
}

}  // namespace UrlPar

#endif  // __cplusplus

#endif
