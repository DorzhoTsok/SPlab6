#include <fstream>
#include <iostream>
#include <windows.h>
#include <string.h>
#include <vector>

using namespace std;

DWORD WINAPI copy_files(PVOID pvParam);
HANDLE mut = CreateMutex(NULL, FALSE, L"Mutex");
HANDLE outputmut = CreateMutex(NULL, FALSE, L"Mutex");

vector<pair<wstring, wstring>> tasks;

int main()
{
    wstring search_files_in_dir = L"*";
    wstring copy_from_path = L"D:\\dir1\\";
    wstring copy_to_path = L"D:\\dir2\\";


    WIN32_FIND_DATAW wfd;
    HANDLE const hFind = FindFirstFileW(
        (LPCWSTR)(copy_from_path + search_files_in_dir).c_str(),
        &wfd
    );
    setlocale(LC_ALL, "");

    if (INVALID_HANDLE_VALUE != hFind)
    {
        do
        {
            wstring ws(wfd.cFileName);
            if (ws == L"." || ws == L"..")
                continue;

            wcout << L"Найден файл: " << ws << endl;

            if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                wcout << L"Это директория. Пропускаем" << endl;
                continue;
            }

            wstring copy_from = copy_from_path + ws;
            wstring copy_to = copy_to_path + ws;

            pair <wstring, wstring> task;
            task.first = copy_from;
            task.second = copy_to;

            tasks.push_back(task);

        } while (NULL != FindNextFile(hFind, &wfd));

        FindClose(hFind);
    }
    const int n = 3;
    DWORD dwThreadId[n], dw;
    HANDLE hThread[n];
    for (size_t i = 0; i < n; i++) {
        hThread[i] = CreateThread(NULL, 0, copy_files, NULL, 0, &dwThreadId[i]);
    }
    WaitForMultipleObjects(n, hThread, TRUE, INFINITE);
    return 0;
}

DWORD WINAPI copy_files(PVOID pvParam) {
    while (tasks.size() != 0) {

        WaitForSingleObject(mut, INFINITE);
        if (tasks.size() == 0) break;
        pair <wstring, wstring> task = tasks[tasks.size() - 1];
        Sleep(100);  // эмулируем сложную работу по обработке задачи
        tasks.pop_back();
        ReleaseMutex(mut);

        wstring copy_from = task.first;
        wstring copy_to = task.second;
        WaitForSingleObject(outputmut, INFINITE);
        wcout << L"Копирование файла из: " << (LPCWSTR)copy_from.c_str() << endl;
        wcout << L"Копирование файла в: " << (LPCWSTR)copy_to.c_str() << endl;
        ReleaseMutex(outputmut);

        bool result = CopyFile(
            (LPCWSTR)copy_from.c_str(),
            (LPCWSTR)copy_to.c_str(),
            FALSE
        );
        WaitForSingleObject(outputmut, INFINITE);
        if (result) {
            wcout << L"Успешно скопировано" << endl;
        }
        else {
            wcout << L"Ошибка в процессе копирования" << endl;
        }
        wcout << endl;
    }
    ReleaseMutex(outputmut);
    DWORD dwResult = 0;
    return dwResult;
}
