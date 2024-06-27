param(
    [double]$increment = 0.002,
    [double]$start = 0.5,
    [double]$end = 0.8,
    [int]$samples = 20
)

function Is-Admin() {
    $currentPrincipal = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent())
    return $currentPrincipal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function main() {
    if (-not (Is-Admin)) {
        Write-Host "error: administrator privileges required"
        return 1
    }

    $iterations = ($end - $start) / $increment
    $totalMs = $iterations * 102 * $samples

    Write-Host "Approximate worst-case estimated time for completion: $([math]::Round($totalMs / 6E4, 2))mins"
    Write-Host "Worst-case is determined by assuming Sleep(1) = ~2ms with 1ms Timer Resolution"
    Write-Host "Start: $($start)ms, End: $($end)ms, Increment: $($increment)ms, Samples: $($samples)"

    Stop-Process -Name "SetTimerResolution" -ErrorAction SilentlyContinue

    Set-Location $PSScriptRoot

    foreach ($dependency in @("SetTimerResolution.exe", "MeasureSleep.exe")) {
        if (-not (Test-Path $dependency)) {
            Write-Host "error: $($dependency) not exists in current directory"
            return 1
        }
    }

    "RequestedResolutionMs,DeltaMs,STDEV" | Out-File results.txt

    for ($i = $start; $i -le $end; $i += $increment) {
        Write-Host "info: benchmarking $($i)ms"

        Start-Process ".\SetTimerResolution.exe" -ArgumentList @("--resolution", ($i * 1E4), "--no-console")

        # unexpected results if there isn't a small delay after setting the resolution
        Start-Sleep 1

        $output = .\MeasureSleep.exe --samples $samples
        $outputLines = $output -split "`n"

        foreach ($line in $outputLines) {
            $avgMatch = $line -match "Avg: (.*)"
            $stdevMatch = $line -match "STDEV: (.*)"

            if ($avgMatch) {
                $avg = $matches[1] -replace "Avg: "
            } elseif ($stdevMatch) {
                $stdev = $matches[1] -replace "STDEV: "
            }
        }

        "$($i), $([math]::Round([double]$avg, 3)), $($stdev)" | Out-File results.txt -Append

        Stop-Process -Name "SetTimerResolution" -ErrorAction SilentlyContinue
    }

    Write-Host "info: results saved in results.txt"
    return 0
}

$_exitCode = main
Write-Host # new line
exit $_exitCode
