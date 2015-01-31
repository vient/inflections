#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <map>
#include <set>
#include <functional>
#include <array>

#include <unicode\regex.h>
#include <unicode\ustdio.h>
#include <unicode\uclean.h>
#include <unicode\locid.h>

#include "inflections.h"

namespace RussianInflections
{
    namespace Service
    {
        bool initFlag = false;

        namespace Exceptions
        {
            class InflateInvalidRuleException : public std::exception
            {
                virtual const char* what() const throw()
                {
                    return "Wrong format of rule";
                }
            } inflateInvalidRuleException;

            class InflateInvalidRequestException : public std::exception
            {
                virtual const char* what() const throw()
                {
                    return "Wrong format of request";
                }
            } inflateInvalidRequestException;
        }

        struct Rule
        {
            UnicodeString ending;
            sex sex_type;
            std::map<grammatical_case, UnicodeString> caseEndings;

            Rule() {}
            Rule(UnicodeString & S, sex sex_type) : sex_type(sex_type)
            {
                UErrorCode status = U_ZERO_ERROR;
                static RegexMatcher spaceSplitter("\\s+", 0, status);
                const int maxWords = 10;
                UnicodeString words[maxWords];

                // After that operation words[0] will be the ending, words[1] will contain sex,
                // words[2] will be "-" and words[3..8] will contain endings for six grammatical cases.
                // split() return number of pieces in result.
                if (spaceSplitter.split(S, words, maxWords, status) != maxWords - 2)
                    throw Exceptions::inflateInvalidRuleException;

                ending = words[0];
                caseEndings[grammatical_case::nominative] = words[2];
                caseEndings[grammatical_case::genitive] = words[3];
                caseEndings[grammatical_case::dative] = words[4];
                caseEndings[grammatical_case::accusative] = words[5];
                caseEndings[grammatical_case::instrumental] = words[6];
                caseEndings[grammatical_case::prepositional] = words[7];
            }

            ~Rule() {}
        };

        std::vector<Rule> maleNameRules, maleSurnameRules, femaleNameRules, femaleSurnameRules, femaleOnlySurnameRules;
        std::set<UnicodeString> maleNames, femaleNames;

        inline void Init()
        {
            if (initFlag)
                return;
            initFlag = true;
            
            std::clog << "Initialization of RussianInflections module:\n";

            UErrorCode status = U_ZERO_ERROR;
            u_init(&status);

            // rulesReadingLambda reads rules from file and saves them to appropriate vector
            auto rulesReadingLambda = [](const fileHandle & handle, std::vector<Rule> & V, sex sexType) -> void
            {
                std::ifstream fin(handle.c_str());
                std::string S;
                char trash, comment = '#';
                fin >> trash >> trash >> trash;     // Reading UTF-8 BOM, we don't need it in first string
                while (getline(fin, S))
                {
                    if (S[0] != comment)        // If first symbol isn't '#'
                        V.push_back(Rule(UnicodeString::fromUTF8(S.c_str()), sexType));
                }
                std::clog << "  Successful read " << V.size() << " strings from " << handle << '\n';
            };

            // namesReadingLambda just saves names from file to set
            auto namesReadingLambda = [](const fileHandle & handle, std::set<UnicodeString> & V) -> void
            {
                std::ifstream fin(handle.c_str());
                std::string S;
                char trash, comment = '#';
                fin >> trash >> trash >> trash;     // Reading UTF-8 BOM, we don't need it in first string
                while (getline(fin, S))
                {
                    if (S[0] != comment)        // If first symbol isn't '#'
                        V.insert(UnicodeString::fromUTF8(S.c_str()).toLower());
                }
                std::clog << "  Successful read " << V.size() << " strings from " << handle << '\n';
            };

            rulesReadingLambda(maleNameRulesFile, maleNameRules, sex::male);
            rulesReadingLambda(maleSurnameRulesFile, maleSurnameRules, sex::male);
            rulesReadingLambda(femaleNameRulesFile, femaleNameRules, sex::female);
            rulesReadingLambda(femaleSurnameRulesFile, femaleSurnameRules, sex::female);
            rulesReadingLambda(femaleOnlySurnameRulesFile, femaleOnlySurnameRules, sex::female);

            namesReadingLambda(maleNameFile, maleNames);
            namesReadingLambda(femaleNameFile, femaleNames);
            std::clog << "----- Done -----\n";
        }
    }
    UnicodeString InflectStringUTF16(const UnicodeString & in, sex sexType = sex::unknown, grammatical_case grammaticalCase = grammatical_case::all)
    {
        Service::Init();

        UErrorCode status = U_ZERO_ERROR;
        RegexMatcher spaceSplitter("\\s+", 0, status), hyphenSplitter("-", 0, status);
        const int maxWords = 10;
        UnicodeString words[maxWords], ans;

        int wordsNum = spaceSplitter.split(in, words, maxWords, status);

        if (sexType == sex::unknown)        // Sex recognizing
        {
            for (int i = 0; sexType == sex::unknown && i < wordsNum; i++)   // Recognizing based on name lists
            {
                UnicodeString Str[maxWords];
                int num = hyphenSplitter.split(words[i], Str, maxWords, status);
                for (int j = 0; j < num; j++)
                {
                    Str[j].toLower();
                    if (Service::maleNames.count(Str[j]))
                    {
                        sexType = sex::male;
                        break;
                    }
                    else if (Service::femaleNames.count(Str[j]))
                    {
                        sexType = sex::female;
                        break;
                    }
                }
            }
            for (int i = 0; sexType == sex::unknown && i < wordsNum; i++)   // Recognizing based on specific female
            {                                                               // inflection rules
                UnicodeString Str[maxWords];
                int num = hyphenSplitter.split(words[i], Str, maxWords, status);
                for (int j = 0; sexType == sex::unknown && j < num; j++)
                {
                    Str[j].toLower();
                    for (auto rule : Service::femaleOnlySurnameRules)
                    {
                        if (Str[j].endsWith(rule.ending))
                        {
                            sexType = sex::female;
                            break;
                        }
                    }
                }
            }
        }
        if (sexType == sex::unknown)
            sexType = sex::male;

        std::set<UnicodeString> names = (sexType == sex::male ? Service::maleNames : Service::femaleNames);
        std::vector<Service::Rule>
            nameRules = (sexType == sex::male ? Service::maleNameRules : Service::femaleNameRules),
            surnameRules = (sexType == sex::male ? Service::maleSurnameRules : Service::femaleSurnameRules);
        UnicodeString Ans[6];

        for (auto word : words)
        {
            UnicodeString Str[maxWords];
            int num = hyphenSplitter.split(word, Str, maxWords, status);
            bool isName = true;

            for (int j = 0; j < num; j++)
            {
                Str[j].toLower();
                isName &= (names.count(Str[j]) != 0);
            }

            if (!Ans[0].isEmpty())
            {
                if (grammaticalCase == grammatical_case::all)
                {
                    for (int i = 0; i < 6; i++)
                        Ans[i].append(" ");
                }
                else
                    Ans[0].append(" ");
            }

            std::vector<Service::Rule> & Rules = (isName ? nameRules : surnameRules);

            for (int j = 0; j < num; j++)
            {
                if (!isName || j == num - 1)
                {
                    bool wasAppropriateRule = false;

                    for (auto rule : Rules)
                    {
                        if (Str[j].endsWith(rule.ending))
                        {
                            wasAppropriateRule = true;

                            Str[j].truncate(Str[j].lastIndexOf(rule.ending));

                            if (grammaticalCase == grammatical_case::all)
                            {
                                for (int k = 0; k < 6; k++)
                                    Ans[k].append(Str[j]);
                                Ans[0].append(rule.caseEndings[grammatical_case::nominative]).toTitle(0, Locale("ru_RU"));
                                Ans[1].append(rule.caseEndings[grammatical_case::genitive]).toTitle(0, Locale("ru_RU"));
                                Ans[2].append(rule.caseEndings[grammatical_case::dative]).toTitle(0, Locale("ru_RU"));
                                Ans[3].append(rule.caseEndings[grammatical_case::accusative]).toTitle(0, Locale("ru_RU"));
                                Ans[4].append(rule.caseEndings[grammatical_case::instrumental]).toTitle(0, Locale("ru_RU"));
                                Ans[5].append(rule.caseEndings[grammatical_case::prepositional]).toTitle(0, Locale("ru_RU"));
                            }
                            else
                            {
                                Ans[0].append(Str[j]).append(rule.caseEndings[grammaticalCase]).toTitle(0, Locale("ru_RU"));
                            }
                            break;
                        }
                    }

                    if (!wasAppropriateRule)
                    {
                        Str[j].toTitle(0, Locale("ru_RU"));
                        if (grammaticalCase == grammatical_case::all)
                        {
                            for (int k = 0; k < 6; k++)
                                Ans[k].append(Str[j]);
                        }
                        else
                            Ans[0].append(Str[j]);
                    }

                    if (j < num - 1)
                    {
                        if (grammaticalCase == grammatical_case::all)
                        {
                            for (int k = 0; k < 6; k++)
                                Ans[k].append("-");
                        }
                        else
                            Ans[0].append("-");
                    }
                }
                else
                {
                    Str[j].toTitle(0, Locale("ru_RU"));
                    if (grammaticalCase == grammatical_case::all)
                    {
                        for (int k = 0; k < 6; k++)
                            Ans[k].append(Str[j]).append("-");
                    }
                    else
                        Ans[0].append(Str[j]).append("-");
                }
            }
        }

        if (grammaticalCase == grammatical_case::all)
        {
            for (int i = 0; i < 5; i++)
                ans.append(Ans[i]).append('\n');
            ans.append(Ans[5]);
        }
        else
            ans = Ans[0];
        return ans;
    }

    std::string InflectStringUTF8(const std::string & in, const sex sexType = sex::unknown, const grammatical_case grammaticalCase = grammatical_case::all)
    {
        UnicodeString S = UnicodeString::fromUTF8(in.c_str());
        std::string ans;
        S = InflectStringUTF16(S, sexType, grammaticalCase);
        S.toUTF8String(ans);
        return ans;
    }
}

