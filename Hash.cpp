#include <cstdio>

#include "D2Helpers.h"
#include "Hash.h"

using namespace std;

char* HashString(char* dataIn, ALG_ID algo) {
    // set up the crypto environment
    HCRYPTPROV provider;
    HCRYPTHASH hash;
    DWORD lenOut = 0;

    // PROV_RSA_AES is the general-purpose provider type with SHA-256/384/512 (the PROV_RSA_FULL providers stop at
    // SHA-1); a verify context needs no key container, which hashing never uses.
    if (!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
        return NULL;
    if (!CryptCreateHash(provider, algo, 0, 0, &hash)) {
        CryptReleaseContext(provider, 0);
        return NULL;
    }

    if (!CryptHashData(hash, (BYTE*)dataIn, strlen(dataIn), 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);
        return NULL;
    }
    if (!CryptGetHashParam(hash, HP_HASHVAL, NULL, &lenOut, 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);
        return NULL;
    }

    BYTE* result = new BYTE[lenOut];
    memset(result, 0, lenOut);
    if (!CryptGetHashParam(hash, HP_HASHVAL, result, &lenOut, 0)) {
        delete[] result;
        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);
        return NULL;
    }

    // tear down the crypto environment
    CryptDestroyHash(hash);
    CryptReleaseContext(provider, 0);

    // it's the caller's responsibility to clean up the result
    char *szBuffer1 = new char[lenOut * 2 + 1], szBuffer2[10] = "";
    memset(szBuffer1, 0, lenOut * 2 + 1);
    for (DWORD i = 0; i < lenOut; i++) {
        sprintf_s(szBuffer2, 10, "%.2x", result[i]);
        strcat_s(szBuffer1, lenOut * 2 + 1, szBuffer2);
    }
    delete[] result;
    return szBuffer1;
}

char* HashFile(wchar_t* file, ALG_ID algo) {
    FILE* fp = NULL;
    // Binary mode: a text-mode read collapses CRLF and the digest would not be the file's bytes.
    if (_wfopen_s(&fp, file, L"rb") != 0 || !fp)
        return NULL;

    HCRYPTPROV provider;
    HCRYPTHASH hash;
    DWORD lenOut = 0;
    // PROV_RSA_AES is the general-purpose provider type with SHA-256/384/512 (the PROV_RSA_FULL providers stop at
    // SHA-1); a verify context needs no key container, which hashing never uses.
    if (!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        fclose(fp);
        return NULL;
    }
    if (!CryptCreateHash(provider, algo, 0, 0, &hash)) {
        CryptReleaseContext(provider, 0);
        fclose(fp);
        return NULL;
    }

    // Fixed-size chunks: the file's size never sizes an allocation, so a file of any length hashes.
    BYTE chunk[4096];
    size_t got;
    bool ok = true;
    while ((got = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (!CryptHashData(hash, chunk, (DWORD)got, 0)) {
            ok = false;
            break;
        }
    }
    if (ferror(fp))
        ok = false;
    fclose(fp);
    if (!ok) {
        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);
        return NULL;
    }

    if (!CryptGetHashParam(hash, HP_HASHVAL, NULL, &lenOut, 0)) {
        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);
        return NULL;
    }

    BYTE* result = new BYTE[lenOut];
    memset(result, 0, lenOut);
    if (!CryptGetHashParam(hash, HP_HASHVAL, result, &lenOut, 0)) {
        delete[] result;
        CryptDestroyHash(hash);
        CryptReleaseContext(provider, 0);
        return NULL;
    }

    // tear down the crypto environment
    CryptDestroyHash(hash);
    CryptReleaseContext(provider, 0);

    // it's the caller's responsibility to clean up the result
    char *szBuffer1 = new char[lenOut * 2 + 1], szBuffer2[10] = "";
    memset(szBuffer1, 0, lenOut * 2 + 1);
    for (DWORD i = 0; i < lenOut; i++) {
        sprintf_s(szBuffer2, 10, "%.2x", result[i]);
        strcat_s(szBuffer1, lenOut * 2 + 1, szBuffer2);
    }
    delete[] result;
    return szBuffer1;
}

char* md5(char* str) {
    return HashString(str, CALG_MD5);
}
char* sha1(char* str) {
    return HashString(str, CALG_SHA1);
}
char* sha256(char* str) {
    return HashString(str, CALG_SHA_256);
}
char* sha384(char* str) {
    return HashString(str, CALG_SHA_384);
}
char* sha512(char* str) {
    return HashString(str, CALG_SHA_512);
}

char* md5_file(wchar_t* file) {
    return HashFile(file, CALG_MD5);
}
char* sha1_file(wchar_t* file) {
    return HashFile(file, CALG_SHA1);
}
char* sha256_file(wchar_t* file) {
    return HashFile(file, CALG_SHA_256);
}
char* sha384_file(wchar_t* file) {
    return HashFile(file, CALG_SHA_384);
}
char* sha512_file(wchar_t* file) {
    return HashFile(file, CALG_SHA_512);
}
