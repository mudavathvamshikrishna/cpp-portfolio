#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// Tell the linker to link against the winhttp library
#pragma comment(lib, "winhttp.lib")

int main() {
    // The URL to download. We'll use a small test file for now.
    const wstring host = L"www.example.com";
    const wstring path = L"/";
    const wstring outputFile = L"downloaded.html";

    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    vector<char> buffer;

    // 1. Initialize WinHTTP session
    hSession = WinHttpOpen(L"WinHTTP File Downloader/1.0",
                           WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                           WINHTTP_NO_PROXY_NAME,
                           WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        cerr << "Error in WinHttpOpen: " << GetLastError() << endl;
        return 1;
    }

    // 2. Specify the target server
    hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
    if (!hConnect) {
        cerr << "Error in WinHttpConnect: " << GetLastError() << endl;
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 3. Create an HTTP request handle
    hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                  NULL, WINHTTP_NO_REFERER,
                                  WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!hRequest) {
        cerr << "Error in WinHttpOpenRequest: " << GetLastError() << endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 4. Send the request
    bResults = WinHttpSendRequest(hRequest,
                                  WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                  WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (!bResults) {
        cerr << "Error in WinHttpSendRequest: " << GetLastError() << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 5. Receive the response
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
        cerr << "Error in WinHttpReceiveResponse: " << GetLastError() << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // Open the output file for writing in binary mode
    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        cerr << "Error opening output file." << endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }

    // 6. Keep reading data until there is none left
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            cerr << "Error in WinHttpQueryDataAvailable: " << GetLastError() << endl;
            break;
        }

        if (dwSize == 0) break; // No more data

        // Allocate a buffer for the data
        buffer.resize(dwSize);

        if (!WinHttpReadData(hRequest, (LPVOID)buffer.data(), dwSize, &dwDownloaded)) {
            cerr << "Error in WinHttpReadData: " << GetLastError() << endl;
            break;
        }

        // Write the data to the file
        outFile.write(buffer.data(), dwDownloaded);
        cout << "Downloaded " << dwDownloaded << " bytes." << endl;

    } while (dwSize > 0);

    cout << "Download complete!" << endl;

    // 7. Clean up
    outFile.close();
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return 0;
}  