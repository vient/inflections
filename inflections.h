
#ifndef INFLECTIONS_H_INCLUDED
#define INFLECTIONS_H_INCLUDED

#include <unicode\ustdio.h>

typedef std::string fileHandle;

namespace RussianInflections
{
	const fileHandle
		maleNameRulesFile = "rules\\maleName.txt",
		maleSurnameRulesFile = "rules\\maleSurname.txt",
		femaleNameRulesFile = "rules\\femaleName.txt",
		femaleSurnameRulesFile = "rules\\femaleSurname.txt",
		femaleOnlySurnameRulesFile = "rules\\femaleOnlySurname.txt",		// For sex recognizing
		maleNameFile = "rules\\maleNameList.txt",
		femaleNameFile = "rules\\femaleNameList.txt";

    enum class sex {male, female, unknown};
    enum class grammatical_case
	{
		nominative,
		genitive,
		dative,
		accusative,
		instrumental,
		prepositional,
		all
	};

    namespace Service
    {
		namespace Exceptions
		{
			class InflateInvalidRuleException ;
			class InflateInvalidRequestException ;
		}

		struct Rule;

        void Init() ;
		Rule & GetAppropriateRule(UnicodeString);
    }

	UnicodeString InflectStringUTF16(const UnicodeString &, const sex, const grammatical_case);
	std::string InflectStringUTF8(const std::string &, const sex, const grammatical_case);
}

#endif // INFLECTIONS_H_INCLUDED

