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

#ifdef WIN32
#define MPQ_TEST_PATH L"d:\\Games\\Installed\\Diablo II\\Patch_D2.mpq"
#else
#define MPQ_TEST_PATH "/Users/kambala/Downloads/2012 005 rus only"
#endif

const int kLanguagesCount = 12;
const char *const kLanguages[kLanguagesCount] = {
    "ENG (English)",    // 0x00
    "ESP (Spanish)",    // 0x01
    "DEU (German)",     // 0x02
    "FRA (French)",     // 0x03
    "POR (Portuguese)", // 0x04
    "ITA (Italian)",    // 0x05
    "JPN (Japanese)",   // 0x06
    "KOR (Korean)",     // 0x07
    "SIN",              // 0x08
    "CHI (Chinese)",    // 0x09
    "POL (Polish)",     // 0x0A
    "RUS (Russian)"     // 0x0B
};

const char *const kUseFileMpqPath = "data\\local\\use";


void print_usage()
{

}

int main (int argc, const char *argv[])
{
    TCHAR mpqPath[MAX_PATH];
    cout << "enter mpq path: ";
    std::wcin.getline(mpqPath, MAX_PATH);
    std::wcout << "entered path: " << mpqPath << endl;

    HANDLE phMPQ = 0;
    DWORD flags = BASE_PROVIDER_FILE | STREAM_PROVIDER_LINEAR | STREAM_FLAG_WRITE_SHARE;
    bool b = SFileOpenArchive(mpqPath, 0, flags, &phMPQ);
    cout << "open result " << b << ", handle " << phMPQ;
    if (phMPQ)
        cout << ", is readonly " << (((TMPQArchive *)phMPQ)->dwFlags & MPQ_FLAG_READ_ONLY) << endl;
    else
    {
        cout << ", error " << GetLastError() << endl;
        return 1;
    }

    HANDLE phUseFile = NULL;
    b = SFileOpenFileEx(phMPQ, kUseFileMpqPath, SFILE_OPEN_FROM_MPQ, &phUseFile);
    cout << "open result " << b << ", handle " << phUseFile << endl;
    char buf;
    DWORD bytes;
    b = SFileReadFile(phUseFile, &buf, 1, &bytes, NULL);
    cout << "read result " << b << ", locale " << (short)buf << endl;
    b = SFileCloseFile(phUseFile);
    cout << "close result " << b << endl;

    HANDLE phNewUseFile = 0;
    b = SFileCreateFile(phMPQ, kUseFileMpqPath, 0, 1, 0, MPQ_FILE_REPLACEEXISTING, &phNewUseFile);
    cout << "create result " << b << ", handle " << phNewUseFile << endl;
    char newLocale = buf ? 0 : 11; // interchange english with russian
    b = SFileWriteFile(phNewUseFile, &newLocale, 1, 0);
    cout << "write result " << b << " new locale " << (short)newLocale << endl;
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
