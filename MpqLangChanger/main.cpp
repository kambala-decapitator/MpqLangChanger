//
//  main.cpp
//  MpqLangChanger
//
//  Created by Andrey Filipenkov on 18.07.13.
//  Copyright (c) 2013 Andrey Filipenkov. All rights reserved.
//

#include "StormLib.h"

#include <iostream>

#define USE_FILE_MPQ_PATH "data\\local\\use"

#ifdef WIN32
#define MPQ_TEST_PATH L"d:\\Games\\Setup\\Windows\\Diablo 2\\Моды\\Median XL\\тест\\Patch_D2.mpq"
#else
#define MPQ_TEST_PATH "/Users/kambala/Downloads/2012 005 rus only"
#endif

using std::cout;
using std::endl;

void print_usage()
{

}

int main (int argc, const char * argv[])
{
    HANDLE phMPQ = 0;
    DWORD flags = BASE_PROVIDER_FILE | STREAM_PROVIDER_LINEAR | STREAM_FLAG_WRITE_SHARE;
    bool b = SFileOpenArchive(MPQ_TEST_PATH, 0, flags, &phMPQ);
    cout << "open result " << b << ", handle " << phMPQ << ", is readonly " << (((TMPQArchive *)phMPQ)->dwFlags & MPQ_FLAG_READ_ONLY) << endl;

    HANDLE phUseFile = NULL;
    b = SFileOpenFileEx(phMPQ, USE_FILE_MPQ_PATH, SFILE_OPEN_FROM_MPQ, &phUseFile);
    cout << "open result " << b << ", handle " << phUseFile << endl;
    char buf;
    DWORD bytes;
    b = SFileReadFile(phUseFile, &buf, 1, &bytes, NULL);
    cout << "read result " << b << ", locale " << (short)buf << endl;
    b = SFileCloseFile(phUseFile);
    cout << "close result " << b << endl;

    HANDLE phNewUseFile = 0;
    b = SFileCreateFile(phMPQ, USE_FILE_MPQ_PATH, 0, 1, 0, MPQ_FILE_REPLACEEXISTING, &phNewUseFile);
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
