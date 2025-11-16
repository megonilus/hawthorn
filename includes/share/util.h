#ifndef megonil_util
#define megonil_util

#if !defined(noret)

#if defined(__GNUC__)
#define noret void __attribute__((noreturn))
#elif defined(_MSC_VER) && _MSC_VER >= 1200
#define noret void __declspec(noreturn)
#else
#define noret void
#endif

#endif

#endif // !megonil_util
