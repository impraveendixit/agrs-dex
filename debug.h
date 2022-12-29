#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#define RED		"\e[31m"
#define GREEN 	"\e[32m"
#define YELLOW 	"\e[33m"
#define WHITE 	"\e[1m"
#define CYAN	"\e[36m"
#define NOCOLOR	"\e[m"

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define FUNCTION __func__
#else
#define FUNCTION __FUNCTION__
#endif

#define ERR_STRING() (errno == 0 ? "UNKNOWN_ERROR" : strerror(errno))

#define ERROR(M, ...) fprintf(stderr, RED"[ERROR]"NOCOLOR "[%s:%d:%s: errno: %s] " M "\n", __FILE__, __LINE__, FUNCTION, ERR_STRING(), ##__VA_ARGS__)
#define WARN(M, ...) fprintf(stderr, YELLOW"[WARN]"NOCOLOR "[%s:%d:%s: errno: %s] " M "\n", __FILE__, __LINE__, FUNCTION, ERR_STRING(), ##__VA_ARGS__)
#define INFO(M, ...) fprintf(stderr, WHITE"[INFO]"NOCOLOR "[%s:%d:%s] " M "\n", __FILE__, __LINE__, FUNCTION, ##__VA_ARGS__)
#define DEBUG(M, ...) fprintf(stderr, CYAN"[DEBUG]"NOCOLOR "[%s:%d:%s] " M "\n", __FILE__, __LINE__, FUNCTION, ##__VA_ARGS__)

#define SENTINEL(M, ...) { ERROR(M, ##__VA_ARGS__); errno = 0; goto error; }

#define CHECK(A, M, ...) if(!(A)) { ERROR(M, ##__VA_ARGS__); errno = 0; goto error; }

#define CHECK_DEBUG(A, M, ...) if(!(A)) { DEBUG(M, ##__VA_ARGS__); errno = 0; goto error; }

#define CHECK_MEM(A) CHECK((A), "Out of memory.")

#define CHECK_EQUAL(A) CHECK_DEBUG((A), "Invalid Match.")

#define CHECK_ARG(A) CHECK_DEBUG((A), "Invalid function argument.")

#endif	/* DEBUG_H_INCLUDED */
