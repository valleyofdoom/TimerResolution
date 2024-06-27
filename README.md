# TimerResolution

[![Downloads](https://img.shields.io/github/downloads/valleyofdoom/TimerResolution/total.svg)](https://github.com/valleyofdoom/TimerResolution/releases)

``MeasureSleep`` is used to measure the precision of Sleep(n). By default, it sits in an infinite loop measuring the sleep deltas, but you can use the ``--samples`` argument to get average and STDEV metrics.

``SetTimerResolution`` is used to request a higher resolution by calling [NtSetTimerResolution](http://undocumented.ntinternals.net/index.html?page=UserMode%2FUndocumented%20Functions%2FTime%2FNtSetTimerResolution.html). As an example to automatically start a hidden instance of the program when the system starts and raise the resolution to 0.5ms, place the binary somewhere safe and create a scheduled task in task scheduler ([instructions](https://www.windowscentral.com/how-create-automated-task-using-task-scheduler-windows-10)) with the command below as an example.

```
C:\SetTimerResolution.exe --resolution 5000 --no-console
```

According to a comment on the [Great Rule Change](https://randomascii.wordpress.com/2020/10/04/windows-timer-resolution-the-great-rule-change) article, on Windows Server 2022+ and Windows 11+, the registry key below can also be used so that requesting a higher resolution is effective on a system-wide level rather than only the calling process. This should only be used for debugging purposes.

```
[HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\kernel]
"GlobalTimerResolutionRequests"=dword:00000001
```

## Precision vs. Resolution Graph

The [micro-adjust-benchmark.ps1](/micro-adjust-benchmark.ps1) script can be used to automate the process of plotting precision against resolutions. The results can be visualized with [chart-studio.plotly.com](https://chart-studio.plotly.com/create).

<img src="/assets/img/results-example.png" width="1000">

## Building

```powershell
git clone https://github.com/valleyofdoom/TimerResolution.git
cd .\TimerResolution\
# x64
$env:VCPKG_DEFAULT_TRIPLET = "x64-windows"
# install dependencies
vcpkg install
# replace "SetTimerResolution" with "MeasureSleep" if desired
MSBuild.exe .\TimerResolution.sln -p:Configuration=Release -p:Platform=x64 -t:SetTimerResolution
```
