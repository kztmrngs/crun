#ifndef CRUN_VERSION_H
#define CRUN_VERSION_H

#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const wchar_t* CRUN_VERSION_STR;
void print_version(const wchar_t* compiler_name_arg);

#ifdef __cplusplus
}
#endif

#endif // CRUN_VERSION_H
