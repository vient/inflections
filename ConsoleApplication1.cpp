#include <iostream>
#include <fstream>
#include <cstdio>

#include <unicode\ustdio.h>
#include <unicode\ustream.h>
#include <unicode\uclean.h>
#include <unicode\bytestream.h>

#include "inflectionsTest.h"

using namespace std;

int main()
{
    try
    {
        RussianInflections::Test::UTF8("C:\\Users\\vient\\Desktop\\TextFile1.txt", "C:\\users\\vient\\Desktop\\TextFile1Ans.txt");
    }
    catch (exception & exc)
    {
        cerr << exc.what();
    }
    return 0;
}