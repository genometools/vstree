#ifndef FILE_H
#define FILE_H

#ifndef _WIN32
#define PATH_SEPARATOR     '/'
#define PATH_SEPARATOR_STR "/"
#define PATH_VAR_SEPARATOR ':'
#else
#define PATH_SEPARATOR     '\\'
#define PATH_SEPARATOR_STR "\\"
#define PATH_VAR_SEPARATOR ';'
#endif

#endif
