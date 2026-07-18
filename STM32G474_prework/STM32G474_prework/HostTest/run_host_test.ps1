param(
  [string]$Uv4Path = 'D:\STM32\Keil_v5\UV4\UV4.exe',
  [string]$PythonPath = ''
)

$ErrorActionPreference = 'Stop'

$HostTestDir = $PSScriptRoot
$MdkDir = Join-Path $HostTestDir 'MDK-ARM'
$ResultsDir = Join-Path $HostTestDir 'results'
$ProjectPath = Join-Path $MdkDir 'HRC_HostTest.uvprojx'
$MapPath = Join-Path $MdkDir 'Objects\HRC_HostTest.map'
$BuildLog = Join-Path $MdkDir 'host_build.log'
$TemplatePath = Join-Path $MdkDir 'run_sim.template.ini'
$IniPath = Join-Path $MdkDir 'run_sim.ini'

if (-not (Test-Path -LiteralPath $Uv4Path)) {
  throw "Keil UV4 not found: $Uv4Path"
}

if (Get-Process UV4 -ErrorAction SilentlyContinue) {
  throw 'Please close existing Keil uVision sessions before running HostTest.'
}

New-Item -ItemType Directory -Path $ResultsDir -Force | Out-Null

$BuildArgs = @('-r', $ProjectPath, '-t', 'HRC_HostTest', '-j4', '-o', $BuildLog)
$Build = Start-Process -FilePath $Uv4Path `
  -ArgumentList $BuildArgs `
  -WorkingDirectory $MdkDir `
  -WindowStyle Hidden `
  -PassThru `
  -Wait

if ($Build.ExitCode -ne 0) {
  Get-Content -LiteralPath $BuildLog
  throw "HostTest build failed with UV4 exit code $($Build.ExitCode)."
}

$MapText = Get-Content -LiteralPath $MapPath -Raw

function Get-SymbolRange {
  param([Parameter(Mandatory)][string]$Name)

  $Pattern = '(?m)^\s*' + [Regex]::Escape($Name) + '\s+0x([0-9A-Fa-f]+)\s+Data\s+(\d+)'
  $Match = [Regex]::Match($MapText, $Pattern)
  if (-not $Match.Success) {
    throw "Cannot find symbol in map: $Name"
  }

  $Start = [Convert]::ToUInt32($Match.Groups[1].Value, 16)
  $Size = [Convert]::ToUInt32($Match.Groups[2].Value, 10)
  [pscustomobject]@{
    Start = ('0x{0:X8}' -f $Start)
    End = ('0x{0:X8}' -f ($Start + $Size - 1))
  }
}

$Trace = Get-SymbolRange 'g_hrc_trace_events'
$Cycles = Get-SymbolRange 'g_hrc_cycle_records'
$Summary = Get-SymbolRange 'g_hrc_summary_record'

$DebugLog = (Join-Path $ResultsDir 'uv4-debug.log')
$TraceFile = (Join-Path $ResultsDir 'trace.hex')
$CyclesFile = (Join-Path $ResultsDir 'cycles.hex')
$SummaryFile = (Join-Path $ResultsDir 'summary.hex')

$Ini = Get-Content -LiteralPath $TemplatePath -Raw
$Replacements = @{
  '@DEBUG_LOG@' = $DebugLog
  '@TRACE_FILE@' = $TraceFile
  '@TRACE_START@' = $Trace.Start
  '@TRACE_END@' = $Trace.End
  '@CYCLES_FILE@' = $CyclesFile
  '@CYCLES_START@' = $Cycles.Start
  '@CYCLES_END@' = $Cycles.End
  '@SUMMARY_FILE@' = $SummaryFile
  '@SUMMARY_START@' = $Summary.Start
  '@SUMMARY_END@' = $Summary.End
}
foreach ($Key in $Replacements.Keys) {
  $Ini = $Ini.Replace($Key, $Replacements[$Key])
}
Set-Content -LiteralPath $IniPath -Value $Ini -Encoding ASCII -NoNewline

$RunStarted = Get-Date
$DebugArgs = @('-d', $ProjectPath, '-t', 'HRC_HostTest', '-j0', '-sg')
$DebugSucceeded = $false
for ($Attempt = 1; $Attempt -le 2; $Attempt++) {
  $Debug = Start-Process -FilePath $Uv4Path `
    -ArgumentList $DebugArgs `
    -WorkingDirectory $MdkDir `
    -WindowStyle Hidden `
    -PassThru

  if ($Debug.WaitForExit(30000)) {
    if ($Debug.ExitCode -ne 0) {
      throw "Keil simulator failed with exit code $($Debug.ExitCode)."
    }
    $DebugSucceeded = $true
    break
  }

  $Debug.Kill()
  $Debug.WaitForExit()
  Start-Sleep -Milliseconds 500
}
if (-not $DebugSucceeded) {
  throw 'Keil simulator did not reach HRC_Sim_TestComplete after two attempts.'
}

foreach ($Required in @($TraceFile, $CyclesFile, $SummaryFile)) {
  $Item = Get-Item -LiteralPath $Required
  if ($Item.LastWriteTime -lt $RunStarted) {
    throw "Simulator output is stale: $Required"
  }
}

if ([string]::IsNullOrWhiteSpace($PythonPath)) {
  $BundledPython = 'C:\Users\18465\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
  if (Test-Path -LiteralPath $BundledPython) {
    $PythonPath = $BundledPython
  } else {
    $PythonCommand = Get-Command python -ErrorAction SilentlyContinue
    if ($null -eq $PythonCommand) {
      throw 'Python 3 is required to decode the Keil SAVE files. Pass -PythonPath explicitly.'
    }
    $PythonPath = $PythonCommand.Source
  }
}

& $PythonPath (Join-Path $HostTestDir 'tools\decode_results.py')
if ($LASTEXITCODE -ne 0) {
  throw 'Independent HRC timing validation failed.'
}

Get-Content -LiteralPath (Join-Path $ResultsDir 'hrc_summary.txt')
