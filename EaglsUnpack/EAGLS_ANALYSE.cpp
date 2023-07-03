#include "EAGLS_ANALYSE.h"
#include <stdio.h>
#include <time.h>
#include <string>

using std::string;

#define PAK_HEAD_SIZE 0xE10

typedef struct _FILE_DESC {
    char FileName[0x18];
    DWORD Offset;
    DWORD Reserved1;
    DWORD Size;
    DWORD Reserved2;
}FILEDESC, * PFILEDESC;

void CEaglsAnalyse::EncryptIdx(BYTE* Buffer, DWORD Size, LPCSTR Key, DWORD Seed) {
    DWORD KeyLen;
    char c;

    KeyLen = strlen(Key);
    srand(Seed);
    for (UINT i = 0; i < Size; i++) {
        c = Key[rand() % KeyLen];
        Buffer[i] ^= c;
    }
}

void CEaglsAnalyse::EncryptPak(BYTE* Buffer, DWORD Size, LPCSTR Key, DWORD Seed) {
    DWORD KeyLen;
    char c;

    KeyLen = strlen(Key);
    srand(Seed);
    for (UINT i = 0; i < Size; i += 2) {
        c = Key[rand() % KeyLen];
        Buffer[i] ^= c;
    }
}

BOOL CEaglsAnalyse::OpenScpack(LPCSTR IdxPath, LPCSTR PakPath) {
    string str;

    str = IdxPath;
    str = str.substr(0, strlen(IdxPath) - 4);
    str += "_upk";
    str += &IdxPath[strlen(IdxPath) - 4];

    if (!CopyFileA(IdxPath, str.c_str(), FALSE)) {
        printf("failed to copy .idx file\n");
        return FALSE;
    }
    m_hFileIdx = CreateFileA(str.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (m_hFileIdx == INVALID_HANDLE_VALUE) {
        printf("failed to open %s\n", str.c_str());
        return FALSE;
    }
    m_hFilePak = CreateFileA(PakPath, GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    if (m_hFilePak == INVALID_HANDLE_VALUE) {
        printf("failed to open %s\n", PakPath);
        CloseHandle(m_hFileIdx);
        return FALSE;
    }
    str = PakPath;
    str = str.substr(0, strlen(PakPath) - 4);
    str += "\\";
    
    if (!CreateDirectoryA(str.c_str(), NULL)) {
        if (GetLastError() != 183) {
            printf("failed to create directory %s\n", str.c_str());
            CloseHandle(m_hFileIdx);
            CloseHandle(m_hFilePak);
            return FALSE;
        }
    }
    memcpy(m_UnpackPath, str.c_str(), str.size() + 1);
    return TRUE;
}

DWORD CEaglsAnalyse::Encrypt(LPCSTR KeyIdx, LPCSTR KeyPak) {
    SIZE_T IdxSize;

    BYTE* IdxBuffer;
    DWORD IdxSeed;

    BYTE* PakBuffer;
    PFILEDESC pFileDesc;
    DWORD PakSeed;
    DWORD PakOffset;
    DWORD PakSize;
    string Path;

    if (!KeyIdx)KeyIdx = arKeyIdx;
    if (!KeyPak)KeyPak = arKeyPak;

    IdxSize = GetFileSize(m_hFileIdx, &IdxSize);
    IdxSize -= sizeof(DWORD);
    printf("IdxSize(exclude seed):%08X\n", IdxSize);
    IdxBuffer = new BYTE[IdxSize];
    SetFilePointer(m_hFileIdx, 0, 0, FILE_BEGIN);
    if (!ReadFile(m_hFileIdx, IdxBuffer, IdxSize, 0, 0)) {
        printf("failed to read idx data\n");
        return 0;
    }
    if (!ReadFile(m_hFileIdx, &IdxSeed, sizeof(DWORD), 0, 0)) {
        printf("failed to read idx seed\n");
        return 0;
    }
    EncryptIdx(IdxBuffer, IdxSize, KeyIdx, IdxSeed);
    SetFilePointer(m_hFileIdx, 0, 0, FILE_BEGIN);
    WriteFile(m_hFileIdx, IdxBuffer, IdxSize, 0, 0);

    for (pFileDesc = (PFILEDESC)IdxBuffer; strlen(pFileDesc->FileName) && pFileDesc->Size; pFileDesc++) {
        PakOffset = pFileDesc->Offset - 0x174B;
        PakSize = pFileDesc->Size;
        SetFilePointer(m_hFilePak, PakOffset, 0, FILE_BEGIN);
        PakBuffer = new BYTE[PakSize];
        memset(PakBuffer, 0, PakSize);
        if (ReadFile(m_hFilePak, PakBuffer, PakSize, 0, 0)) {
            __asm {
                push eax
                mov eax, pFileDesc
                mov eax, [eax]pFileDesc.Size
                dec eax
                add eax, PakBuffer
                mov al, [eax]
                movsx eax, al
                mov PakSeed, eax
                pop eax
            }
            printf("Offset:%08X Size:%08X Seed:%08X Name:%s\n", PakOffset, PakSize, PakSeed, pFileDesc->FileName);
            EncryptPak(&PakBuffer[PAK_HEAD_SIZE], PakSize - PAK_HEAD_SIZE - 1, KeyPak, PakSeed);

            Path = m_UnpackPath;
            Path += pFileDesc->FileName;
            m_hFileSub = CreateFileA(Path.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
            if (m_hFileSub != INVALID_HANDLE_VALUE) {
                WriteFile(m_hFileSub, PakBuffer, PAK_HEAD_SIZE, 0, 0);
                CloseHandle(m_hFileSub);
            }

            Path = m_UnpackPath;
            Path += pFileDesc->FileName;
            Path = Path.substr(0, Path.size() - 4);
            Path += ".txt";
            m_hFileSub = CreateFileA(Path.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
            if (m_hFileSub != INVALID_HANDLE_VALUE) {
                WriteFile(m_hFileSub, &PakBuffer[PAK_HEAD_SIZE], PakSize - PAK_HEAD_SIZE - 2, 0, 0);
                CloseHandle(m_hFileSub);
            }
        }
        delete[]PakBuffer;
        
    }
    delete[]IdxBuffer;

    CloseHandle(m_hFileIdx);
    CloseHandle(m_hFilePak);
    return IdxSize;
}

CEaglsAnalyse::CEaglsAnalyse() {
    m_hFileIdx = 0;
    m_hFilePak = 0;
    m_hFileSub = 0;
    memset(m_UnpackPath, 0, sizeof(m_UnpackPath));
}

CEaglsAnalyse::~CEaglsAnalyse() {
    if (m_hFileIdx != 0) {
        CloseHandle(m_hFileIdx);
    }
    if (m_hFilePak != 0) {
        CloseHandle(m_hFilePak);
    }
    if (m_hFileSub != 0) {
        CloseHandle(m_hFileSub);
    }
}

BOOL CEaglsAnalyse::OpenUnpack(LPCSTR DirectoryPath) {
    string str;

    str = DirectoryPath;
    if (str[str.size() - 1] != '\\') {
        str += '\\';
    }
    str += "*.dat";
    memcpy(m_UnpackPath, str.c_str(), str.size() + 1);
    return TRUE;
}

void CEaglsAnalyse::Pack(LPCSTR FilePath, LPCSTR FileName, DWORD IdxSize, DWORD IdxSeed, LPCSTR KeyIdx, LPCSTR KeyPak) {
    string str;
    HANDLE hFind;
    WIN32_FIND_DATAA FindData;
    FILEDESC FileDesc;

    BYTE HeadBuffer[PAK_HEAD_SIZE];
    BYTE* PakBuffer;
    DWORD PakSeed;
    DWORD PakSize;
    DWORD PakOffset;
    char c;

    BYTE* IdxBuffer;

    if (!KeyIdx)KeyIdx = arKeyIdx;
    if (!KeyPak)KeyPak = arKeyPak;

    memset(HeadBuffer, 0, sizeof(HeadBuffer));

    str = FilePath;
    if (str.size() && str[str.size() - 1] != '\\') {
        str += '\\';
    }
    str += FileName;

    m_hFileIdx = CreateFileA((str + ".idx").c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
    if (m_hFileIdx == INVALID_HANDLE_VALUE) {
        printf("failed to create %s\n", (str + ".idx").c_str());
        return;
    }
    m_hFilePak = CreateFileA((str + ".pak").c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
    if (m_hFilePak == INVALID_HANDLE_VALUE) {
        printf("failed to create %s\n", (str + ".pak").c_str());
        CloseHandle(m_hFileIdx);
        return;
    }

    hFind = FindFirstFileA(m_UnpackPath, &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("failed to find *.dat at %s\n", ((string)m_UnpackPath).substr(0, strlen(m_UnpackPath) - 5));
    }
    PakOffset = 0;
    do {
        if (FindData.nFileSizeLow != PAK_HEAD_SIZE) {
            printf("%s error:size != %d\n", FindData.cFileName, PAK_HEAD_SIZE);
        }
        str = ((string)m_UnpackPath).substr(0, strlen(m_UnpackPath) - 5) + FindData.cFileName;
        m_hFileSub = CreateFileA(str.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (m_hFileSub == INVALID_HANDLE_VALUE) {
            printf("failed to open %s\n", str.c_str());
            continue;
        }
        ReadFile(m_hFileSub, HeadBuffer, PAK_HEAD_SIZE, 0, 0);
        CloseHandle(m_hFileSub);

        str = str.substr(0, str.size() - 4);
        str += ".txt";
        m_hFileSub = CreateFileA(str.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        if (m_hFileSub == INVALID_HANDLE_VALUE) {
            printf("failed to open %s\n", str.c_str());
            continue;
        }
        PakSize = GetFileSize(m_hFileSub, &PakSize);
        PakSize += 2;
        PakBuffer = new BYTE[PakSize];
        ReadFile(m_hFileSub, PakBuffer, PakSize - 2, 0, 0);
        PakBuffer[PakSize - 1] = time(0);
        PakBuffer[PakSize - 2] = ' ';
        __asm {
            push eax
            mov eax, PakSize
            dec eax
            add eax, PakBuffer
            mov al, [eax]
            movsx eax, al
            mov PakSeed, eax
            pop eax
        }
        EncryptPak(PakBuffer, PakSize - 2, KeyPak, PakSeed);

        memset(FileDesc.FileName, 0, sizeof(FileDesc.FileName));
        memcpy(FileDesc.FileName, FindData.cFileName, strlen(FindData.cFileName));
        FileDesc.Offset = PakOffset + 0x174B;
        FileDesc.Reserved1 = 0;
        FileDesc.Size = PakSize + PAK_HEAD_SIZE;
        FileDesc.Reserved2 = 0;

        WriteFile(m_hFileIdx, &FileDesc, sizeof(FileDesc), 0, 0);
        WriteFile(m_hFilePak, HeadBuffer, PAK_HEAD_SIZE, 0, 0);
        WriteFile(m_hFilePak, PakBuffer, PakSize, 0, 0);
        
        PakOffset += (PakSize + PAK_HEAD_SIZE);
        delete[]PakBuffer;

    } while (FindNextFileA(hFind, &FindData));

    IdxBuffer = new BYTE[IdxSize];
    memset(IdxBuffer, 0, IdxSize);
    SetFilePointer(m_hFileIdx, 0, 0, FILE_BEGIN);
    ReadFile(m_hFileIdx, IdxBuffer, IdxSize, 0, 0);
    EncryptIdx(IdxBuffer, IdxSize, KeyIdx, IdxSeed);
    SetFilePointer(m_hFileIdx, 0, 0, FILE_BEGIN);
    WriteFile(m_hFileIdx, IdxBuffer, IdxSize, 0, 0);
    delete[]IdxBuffer;
    WriteFile(m_hFileIdx, &IdxSeed, sizeof(DWORD), 0, 0);

    CloseHandle(m_hFileIdx);
    CloseHandle(m_hFilePak);
}