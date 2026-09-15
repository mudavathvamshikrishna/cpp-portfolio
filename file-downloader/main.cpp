#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#pragma comment(lib, "winhttp.lib")

wstring toWString(const string& s) {
    if (s.empty()) return wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0);
    wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &result[0], size);
    return result;
}

bool parseUrl(const string& url, wstring& host, wstring& path, INTERNET_PORT& port, bool& useHttps) {
    string remainder;
    if (url.rfind("https://", 0) == 0) {
        useHttps = true;
        remainder = url.substr(8);
        port = INTERNET_DEFAULT_HTTPS_PORT;
    } else if (url.rfind("http://", 0) == 0) {
        useHttps = false;
        remainder = url.substr(7);
        port = INTERNET_DEFAULT_HTTP_PORT;
    } else {
        return false;
    }

    size_t slashPos = remainder.find('/');
    string hostPart = (slashPos == string::npos) ? remainder : remainder.substr(0, slashPos);
    string pathPart = (slashPos == string::npos) ? "/" : remainder.substr(slashPos);

    host = toWString(hostPart);
    path = toWString(pathPart);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: main.exe <url> <output_file>" << endl;
        cout << "Example: main.exe http://example.com index.html" << endl;
        return 1;
    }

    string url = argv[1];
    string outputFile = argv[2];

    wstring host, path;
    INTERNET_PORT port;
    bool useHttps = false;

    if (!parseUrl(url, host, path, port, useHttps)) {
        cerr << "Error: URL must start with http:// or https://" << endl;
        return 1;
    }

    cout << "Host: " << string(host.begin(), host.end()) << endl;
    cout << "Path: " << string(path.begin(), path.end()) << endl;

    HINTERNET hSession = WinHttpOpen(L"WinHTTP File Downloader/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        cerr << "WinHttpOpen failed: " << GetLastError() << endl;
        return 1;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        cerr << "WinHttpConnect failed: " << GetLastError() << endl;
        WinHttpCloseHandle(hSession);
        return 1;
    }

    DWORD flags = useHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        cerr << "WinHttpOpenRequest failed: " << GetLastError() << endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        cerr << "WinHttpSendRequest failed: " << GetLastError() << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        cerr << "WinHttpReceiveResponse failed: " << GetLastError() << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        cerr << "Cannot open output file: " << outputFile << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    vector<char> buffer;
    long long totalBytes = 0;

    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) break;
        if (dwSize == 0) break;

        buffer.resize(dwSize);
        if (!WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded)) break;

        outFile.write(buffer.data(), dwDownloaded);
        totalBytes += dwDownloaded;
    } while (dwSize > 0);

    outFile.close();
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    cout << "Downloaded " << totalBytes << " bytes to " << outputFile << endl;
    return 0;
}