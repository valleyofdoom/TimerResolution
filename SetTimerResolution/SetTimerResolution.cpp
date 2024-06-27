#include "args.hxx"
#include <iomanip>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>

extern "C" NTSYSAPI NTSTATUS NTAPI NtQueryTimerResolution(PULONG MinimumResolution, PULONG MaximumResolution, PULONG CurrentResolution);
extern "C" NTSYSAPI NTSTATUS NTAPI NtSetTimerResolution(ULONG DesiredResolution, BOOLEAN SetResolution, PULONG CurrentResolution);

typedef BOOL(WINAPI* PSET_PROCESS_INFORMATION)(HANDLE, PROCESS_INFORMATION_CLASS, LPVOID, DWORD);

int CountProcessInstances(const std::wstring& process_name) {
    int count = 0;

    HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        return count;
    }

    PROCESSENTRY32 process_entry;
    process_entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(handle, &process_entry)) {
        CloseHandle(handle);
        return count;
    }

    do {
        std::wstring current_process(process_entry.szExeFile);
        if (current_process == process_name) {
            count++;
        }
    } while (Process32Next(handle, &process_entry));

    CloseHandle(handle);
    return count;
}

int main(int argc, char** argv) {
    std::string version = "1.0.0";

    args::ArgumentParser parser("SetTimerResolution Version " + version + " - GPLv3\nGitHub - https://github.com/valleyofdoom\n");
    args::HelpFlag help(parser, "", "display this help menu", { "help" });
    args::ValueFlag<int> resolution(parser, "", "specify the desired resolution in 100-ns units", { "resolution" }, args::Options::Required);
    args::Flag no_console(parser, "", "hide the console window", { "no-console" });

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what();
        std::cerr << parser;
        return 1;
    } catch (args::ValidationError e) {
        std::cerr << e.what();
        std::cerr << parser;
        return 1;
    }

    if (CountProcessInstances(L"SetTimerResolution.exe") > 1) {
        std::cerr << "Another instance of SetTimerResolution is already running. Close all instances and try again\n";
        return 1;
    }

    if (no_console) {
        FreeConsole();
    }

    ULONG minimum_resolution, maximum_resolution, current_resolution;

    HMODULE kernel32 = LoadLibrary(L"kernel32.dll");

    if (!kernel32) {
        std::cerr << "LoadLibrary failed\n";
        return 1;
    }

    PSET_PROCESS_INFORMATION SetProcessInformation = (PSET_PROCESS_INFORMATION)GetProcAddress(kernel32, "SetProcessInformation");

    // does not exist in Windows 7
    if (SetProcessInformation) {
        PROCESS_POWER_THROTTLING_STATE PowerThrottling;
        RtlZeroMemory(&PowerThrottling, sizeof(PowerThrottling));

        PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
        PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_IGNORE_TIMER_RESOLUTION;
        PowerThrottling.StateMask = 0;

        SetProcessInformation(GetCurrentProcess(), ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
    }

    if (NtQueryTimerResolution(&minimum_resolution, &maximum_resolution, &current_resolution)) {
        std::cerr << "NtQueryTimerResolution failed\n";
        return 1;
    }

    if (NtSetTimerResolution(args::get(resolution), true, &current_resolution)) {
        std::cerr << "NtSetTimerResolution failed\n";
        return 1;
    }

    std::cout << "Resolution set to: " << (current_resolution / 10000.0) << "ms\n";
    Sleep(INFINITE);
}
