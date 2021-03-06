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
#define LANGUAGE_INFO(langId) " 0x0" << std::hex << static_cast<short>(langId) << " " << kLanguages[langId] << endl

#define EXIT_WITH_ERROR(error_text, f) \
{\
    cout << error_text << endl << endl; \
    f(); \
    return EXIT_FAILURE;\
}
#define EXIT_AND_PRINT_HELP(error_text)  EXIT_WITH_ERROR(error_text, print_help)
#define EXIT_AND_PRINT_USAGE(error_text) EXIT_WITH_ERROR(error_text, print_usage)

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
    if (langData.language)
        o << " (" << langData.language << ")";
    return o;
}

const char *const kUseFileMpqPath = "data\\local\\use", *const kOptionValueDelimiter = "=", *const kIndentation = "    ";


void print_usage()
{
    const char *kUsageStartText = "usage: "APP_NAME" ";

    cout << kUsageStartText << "-l=[language ID (hex) or 3-letter key] mpq_path" << endl;
    cout << kIndentation << "set new MPQ language" << endl << "OR" << endl;

    cout << kUsageStartText << "-p mpq_path" << endl;
    cout << kIndentation << "print current MPQ language" << endl;
}

void print_help()
{
    cout << "available languages:\n";
    cout << kIndentation << "ID    Key  Language\n";
    cout << kIndentation << "-------------------\n";
    for (int i = 0; i < kLanguagesCount; ++i)
    {
        LanguageData data = kLanguages[i];
        cout << kIndentation << "0x0" << std::hex << i << "  " << data.languageKey;
        if (data.language)
            cout << "  " << data.language;
        cout << endl;
    }
}

int main(int argc, const char *argv[])
{
    if (argc < 3)
    {
        print_usage();
        cout << endl;
        print_help();
        return EXIT_FAILURE;
    }

    bool justPrintCurrentLanguage = false;
    char langId;

    char *option = strtok(const_cast<char *>(argv[1]), kOptionValueDelimiter);
    if (!option)
        EXIT_AND_PRINT_USAGE("option missing");

    if (!strcmp(option, "-p"))
        justPrintCurrentLanguage = true;
    else if (!strcmp(option, "-l"))
    {
        char *langParam = strtok(0, kOptionValueDelimiter);
        if (!langParam)
            EXIT_AND_PRINT_USAGE("language value is missing");

        bool isUnsupportedLanguage = true;
        switch (strlen(langParam)) {
            case 3: // probably 3-letter key
                for (int i = 0; i < kLanguagesCount; ++i)
                {
                    if (!strcmp(langParam, kLanguages[i].languageKey))
                    {
                        langId = i;
                        isUnsupportedLanguage = false;
                        break;
                    }
                }
                break;
            case 4: // probably hex ID
            {
#ifndef WIN32
                if (strcasestr(langParam, "0x0") != langParam)
#else
                if (strstr(langParam, "0x0") != langParam && strstr(langParam, "0X0") != langParam)
#endif
                    EXIT_AND_PRINT_HELP("wrong hex format");

                char langChar = toupper(langParam[3]);
                if (isdigit(langChar))
                    langId = atoi(&langChar);
                else if (isxdigit(langChar))
                {
                    langChar -= 17; // map to 0-9
                    langId = 10 + atoi(&langChar);
                }
                else
                    EXIT_AND_PRINT_HELP("it's not a hex number");

                isUnsupportedLanguage = langId >= kLanguagesCount;
            }
                break;
            default:
                break;
        }

        if (isUnsupportedLanguage)
            EXIT_AND_PRINT_HELP("this language isn't supported");

        cout << "new MPQ language will be" << LANGUAGE_INFO(langId);
    }
    else
        EXIT_AND_PRINT_USAGE("illegal option: " << option);

    const TCHAR *mpqPath = argv[2];
    HANDLE hMPQ = 0;
    if (!SFileOpenArchive(mpqPath, 0, BASE_PROVIDER_FILE | STREAM_PROVIDER_LINEAR | STREAM_FLAG_WRITE_SHARE, &hMPQ))
    {
        cout << "error opening MPQ '" << mpqPath << "': " << GetLastError() << "\neither invalid path or unsupported file." << endl;
        return EXIT_FAILURE;
    }

    if (justPrintCurrentLanguage)
    {
        HANDLE hUseFile = 0;
        if (SFileOpenFileEx(hMPQ, kUseFileMpqPath, SFILE_OPEN_FROM_MPQ, &hUseFile) && SFileReadFile(hUseFile, &langId, 1, 0, 0))
            cout << "current MPQ language is" << LANGUAGE_INFO(langId);
        else
            cout << "error reading MPQ language: " << GetLastError() << endl;
        SFileCloseFile(hUseFile);
    }
    else
    {
        HANDLE hNewUseFile = 0;
        if (!SFileCreateFile(hMPQ, kUseFileMpqPath, 0, 1, 0, MPQ_FILE_REPLACEEXISTING, &hNewUseFile) || !SFileWriteFile(hNewUseFile, &langId, 1, 0))
            cout << "error setting new MPQ language: " << GetLastError() << endl;
        SFileFinishFile(hNewUseFile);
    }

    SFileCloseArchive(hMPQ);
    return 0;
}
