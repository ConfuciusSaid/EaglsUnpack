#pragma once
#include <windows.h>

const char arKeyIdx[46] = {
    0x31 ,0x71 ,0x61 ,0x7A ,0x32 ,0x77 ,0x73 ,0x78 ,0x33 ,0x65 ,0x64 ,0x63 ,
    0x34 ,0x72 ,0x66 ,0x76 ,0x35 ,0x74 ,0x67 ,0x62 ,0x36 ,0x79 ,0x68 ,0x6E ,
    0x37 ,0x75 ,0x6A ,0x6D ,0x38 ,0x69 ,0x6B ,0x2C ,0x39 ,0x6F ,0x6C ,0x2E ,
    0x30 ,0x70 ,0x3B ,0x2F ,0x2D ,0x40 ,0x3A ,0x5E ,0x5B ,0x5D
};

const char arKeyPak[13] = "EAGLS_SYSTEM";

class CEaglsAnalyse
{
private:
    HANDLE m_hFileIdx;
    HANDLE m_hFilePak;
    HANDLE m_hFileSub;

    char m_UnpackPath[0x100];

    void EncryptIdx(BYTE* Buffer, DWORD Size, LPCSTR Key, DWORD Seed);
    void EncryptPak(BYTE* Buffer, DWORD Size, LPCSTR Key, DWORD Seed);
public:
    CEaglsAnalyse();
    ~CEaglsAnalyse();

    BOOL OpenScpack(LPCSTR IdxPath, LPCSTR PakPath);
    DWORD Encrypt(LPCSTR KeyIdx = 0, LPCSTR KeyPak = 0);    // return IdxSize(exclude seed)

    BOOL OpenUnpack(LPCSTR DirectoryPath);
    void Pack(LPCSTR FilePath, LPCSTR FileName, DWORD IdxSize, DWORD IdxSeed, LPCSTR KeyIdx = 0, LPCSTR KeyPak = 0);
};