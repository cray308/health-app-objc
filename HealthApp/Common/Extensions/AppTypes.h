#ifndef AppTypes_h
#define AppTypes_h

#if defined(__LP64__) && __LP64__
typedef long HAInt;
#define LHASymbol "q"
#define MUTEX_CHARS "[56c]"
#define COND_CHARS "[40c]"
#else
typedef int HAInt;
#define LHASymbol "l"
#define MUTEX_CHARS "[40c]"
#define COND_CHARS "[24c]"
#endif

#endif /* AppTypes_h */
