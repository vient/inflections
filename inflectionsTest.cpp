#include <fstream>
#include <unicode\regex.h>
#include <unicode\ustdio.h>
#include <unicode\uclean.h>
#include <unicode\locid.h>

#include "inflectionsTest.h"

namespace RussianInflections
{
    namespace Test
    {
        sex WhatSexIsIt(UnicodeString S)
        {
            UnicodeString male("ì"), female("æ");
            S.toLower(Locale("ru_RU"));
            if (S == male)
                return sex::male;
            if (S == female)
                return sex::female;
            return sex::unknown;
        }

        grammatical_case WhatGrammaticalCaseIsIt(UnicodeString S)
        {
            S.toLower(Locale("ru_RU"));
            if (S == UnicodeString("è"))
                return grammatical_case::nominative;
            if (S == UnicodeString("ð"))
                return grammatical_case::genitive;
            if (S == UnicodeString("ä"))
                return grammatical_case::dative;
            if (S == UnicodeString("â"))
                return grammatical_case::accusative;
            if (S == UnicodeString("ò"))
                return grammatical_case::instrumental;
            if (S == UnicodeString("ï"))
                return grammatical_case::prepositional;
            return grammatical_case::all;
        }

        void UTF8(fileHandle & in, fileHandle & out)
        {
            std::ifstream fin(in.c_str());
            std::ofstream fout(out.c_str());
            std::string S;

            UErrorCode status = U_ZERO_ERROR;
            RegexMatcher spaceSplitter("\\s+", 0, status);

            char trash, comment = '#';

            fin >> trash >> trash >> trash;     // Reading UTF-8 BOM

            while (getline(fin, S))
            {
                if (!S.empty() && S[0] != comment)
                {
                    const int maxWords = 10;
                    UnicodeString US = UnicodeString::fromUTF8(S.c_str()), words[maxWords];

                    const int wordsNum = spaceSplitter.split(US, words, maxWords, status);

                    sex sexType = WhatSexIsIt(words[wordsNum - 2]);
                    grammatical_case grammaticalCase = WhatGrammaticalCaseIsIt(words[wordsNum - 1]);
                    US.remove();
                    for (int i = 0; i < wordsNum - 2; i++)      // Removing sex and gr. case from string
                        US.append(words[i]).append(" ");

                    if (sexType == sex::unknown)
                        sexType = WhatSexIsIt(words[wordsNum - 1]);

                    if (WhatSexIsIt(words[wordsNum - 1]) != sex::unknown)
                        US.append(words[wordsNum - 2]).append(" ");
                    else if (sexType == sex::unknown)
                        US.append(words[wordsNum - 2]).append(" ").append(words[wordsNum - 1]).append(" ");

                    S.clear();
                    InflectStringUTF16(US, sexType, grammaticalCase).toUTF8String(S);
                    fout << S << std::endl << std::endl;
                }
            }
        }

        void UTF8(char * in, char * out)
        {
            UTF8(fileHandle(in), fileHandle(out));
        }
    }
}