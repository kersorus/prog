#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#define CHECKR(condition,message)\
        if(condition)\
        {\
            perror(message);\
            exit(EXIT_FAILURE);\
        }

#endif
