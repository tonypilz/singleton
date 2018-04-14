#pragma once


#ifdef __has_cpp_attribute
    #ifndef __cpp_exceptions
        #define EXCEPTIONS_DISABLED
    #endif
#else

    //gcc has its own macro prior to supporting __has_cpp_attribute
    #if (!defined(__clang__)) && (__GNUC__>= 4) && (__GNUC_MINOR__ >= 7) && (__GNUC_PATCHLEVEL__ >=1)
        #ifndef __EXCEPTIONS
            #define EXCEPTIONS_DISABLED
        #endif
    #else
        #warning Cannot detect if exceptions are enabled, so enabling them by default. They can be manually disabled by defining 'EXCEPTIONS_DISABLED'
    #endif // __GNUC__

#endif // __has_cpp_attribute
