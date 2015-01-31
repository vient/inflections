#ifndef INFLECTIONSTEST_H_INCLUDED
#define INFLECTIONSTEST_H_INCLUDED

#include "inflections.h"

namespace RussianInflections
{
    namespace Test
    {
        sex WhatSexIsIt(UnicodeString S);
        grammatical_case WhatGrammaticalCaseIsIt(UnicodeString S);

        void UTF8(fileHandle & in, fileHandle & out);
        void UTF8(char * in, char * out);
    }
}

#endif // INFLECTIONSTEST_H_INCLUDED