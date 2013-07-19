//
//  main.cpp
//  MpqLangChanger
//
//  Created by Andrey Filipenkov on 18.07.13.
//  Copyright (c) 2013 Andrey Filipenkov. All rights reserved.
//

#include "StormLib.h"

#include <iostream>

using std::cout;
using std::endl;

#define APP_NAME "MpqLangChanger"

#ifdef WIN32
#define MPQ_TEST_PATH L"d:\\Games\\Installed\\Diablo II\\Patch_D2.mpq"
#else
#define MPQ_TEST_PATH "/Users/kambala/Downloads/2012 005 rus only"
#endif

const int kLanguagesCount = 12;
const struct LanguageData { const char *languageKey, *language; } kLanguages[kLanguagesCount] = {
    {"ENG", "English"},    // 0x00
    {"ESP", "Spanish"},    // 0x01
    {"DEU", "German"},     // 0x02
    {"FRA", "French"},     // 0x03
    {"POR", "Portuguese"}, // 0x04
    {"ITA", "Italian"},    // 0x05
    {"JPN", "Japanese"},   // 0x06
    {"KOR", "Korean"},     // 0x07
    {"SIN", 0},            // 0x08
    {"CHI", "Chinese"},    // 0x09
    {"POL", "Polish"},     // 0x0A
    {"RUS", "Russian"}     // 0x0B
};
std::ostream &operator <<(std::ostream &o, const LanguageData &langData)
{
    o << langData.languageKey;
    if (langData.language) {
        o << " (" << langData.language << ")";
    }
    return o;
}

const char *const kUseFileMpqPath = "data\\local\\use", *const kOptionValueDelimeter = "=";


void print_usage()
{
    cout << "usage: "APP_NAME
#ifdef WIN32
    ".exe"
#endif
    " -l=<language ID (hex) or 3-letter key> <MPQ path>" << endl;
}

void print_help()
{
    cout << "help\n";
    return;
    const char *indent = "    ";
    cout << "available languages:\n";
    cout << indent << "ID    Key  Language\n";
    cout << indent << "-------------------\n";
    for (int i = 0; i < kLanguagesCount; ++i) {
        LanguageData data = kLanguages[i];
        cout << indent << "0x0" << std::hex << i << "  " << data.languageKey;
        if (data.language) {
            cout << "  " << data.language;
        }
        cout << endl;
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 3) {
        print_usage();
        cout << endl;
        print_help();
        return 1;
    }

    char *option = strtok(const_cast<char *>(argv[1]), kOptionValueDelimeter);
    if (strcmp(option, "-l")) {
        cout << "illegal option: " << option << endl;
        print_usage();
        return 2;
    }
    char *langParam = strtok(0, kOptionValueDelimeter);
    if (!langParam) {
        cout << "language value is missing." << endl;
        print_usage();
        return 3;
    }
    cout << "got lang " << langParam << endl;
    if (strlen(langParam) != 4 || strcasestr(langParam, "0x0") != langParam) {
        cout << "wrong hex format. ";
        print_help();
        return 4;
    }
    char langChar = toupper(langParam[3]);
    char langId;
    if (isdigit(langChar))
    {
        langId = atoi(&langChar);
    }
    else if (isxdigit(langChar))
    {
        langChar -= 17; // map to 0-9
        langId = 10 + atoi(&langChar);
    }
    else
    {
        cout << "it's not a hex number. ";
        print_help();
        return 5;
    }
    if (langId >= kLanguagesCount) {
        cout << "this language isn't supported. ";
        print_help();
        return 6;
    }

    const TCHAR *mpqPath = argv[2];
    cout << "entered path: " << mpqPath << endl;

    HANDLE phMPQ = 0;
    DWORD flags = BASE_PROVIDER_FILE | STREAM_PROVIDER_LINEAR | STREAM_FLAG_WRITE_SHARE;
    bool b = SFileOpenArchive(mpqPath, 0, flags, &phMPQ);
    cout << "open result " << b << ", handle " << phMPQ << ", ";
    if (phMPQ)
        cout << "is readonly " << (((TMPQArchive *)phMPQ)->dwFlags & MPQ_FLAG_READ_ONLY) << endl;
    else
    {
        cout << "error " << GetLastError() << endl;
        return 7;
    }

    HANDLE phUseFile = NULL;
    b = SFileOpenFileEx(phMPQ, kUseFileMpqPath, SFILE_OPEN_FROM_MPQ, &phUseFile);
    cout << "open result " << b << ", handle " << phUseFile << endl;
    char buf;
    DWORD bytes;
    b = SFileReadFile(phUseFile, &buf, 1, &bytes, NULL);
    cout << "read result " << b << ", locale " << (short)buf << " " << kLanguages[buf] << endl;
    b = SFileCloseFile(phUseFile);
    cout << "close result " << b << endl;

    HANDLE phNewUseFile = 0;
    b = SFileCreateFile(phMPQ, kUseFileMpqPath, 0, 1, 0, MPQ_FILE_REPLACEEXISTING, &phNewUseFile);
    cout << "create result " << b << ", handle " << phNewUseFile << endl;
    b = SFileWriteFile(phNewUseFile, &langId, 1, 0);
    cout << "write result " << b << " new locale " << (short)langId << " " << kLanguages[langId] << endl;
    if (phNewUseFile)
    {
        b = SFileFinishFile(phNewUseFile);
        cout << "finish result " << b << endl;
    }

    b = SFileCompactArchive(phMPQ, 0, false);
    cout << "compact result " << b << ", error " << GetLastError() << endl;
    b = SFileCloseArchive(phMPQ);
    cout << "close result " << b << endl;
    
    return 0;
}
