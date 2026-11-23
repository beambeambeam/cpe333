param([string]$SecondDrive = 'D:')
$ErrorActionPreference = 'Stop'
if (-not $SecondDrive) { $SecondDrive = 'D:' }
if ($SecondDrive -notmatch '^[D-Zd-z]:$') { throw 'Use a second drive such as D:.' }
$SecondDrive = $SecondDrive.ToUpperInvariant()
$principal = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw 'Run windows-links.cmd from an Administrator Command Prompt. No system settings are changed.'
}
$results = Join-Path $PSScriptRoot 'results'
New-Item -ItemType Directory -Force -Path $results | Out-Null
$runId = 'ps12-' + [guid]::NewGuid().ToString('N').Substring(0,8)
$work = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) ('Temp\' + $runId)
if ([IO.Path]::GetPathRoot($work) -ne 'C:\') { throw 'The source workspace must be on C:.' }
$cross = Join-Path ($SecondDrive + '\') $runId
foreach ($path in @($work, $cross)) {
    if (Test-Path -LiteralPath $path) { throw "Refusing to reuse $path" }
}
$sections = [Collections.Generic.List[object]]::new()
$checks = [Collections.Generic.List[object]]::new()
$junctions = [Collections.Generic.List[string]]::new()
$script:cwd = $work
function Section([string]$id, [string]$title) {
    $script:current = [pscustomobject]@{ id=$id; title=$title; commands=[Collections.Generic.List[object]]::new() }
    $sections.Add($script:current)
}
function Run([string]$command, [bool]$expectSuccess=$true) {
    $start = [Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $env:ComSpec
    $start.Arguments = '/d /c ' + $command
    $start.WorkingDirectory = $script:cwd
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $process = [Diagnostics.Process]::Start($start)
    $outTask = $process.StandardOutput.ReadToEndAsync()
    $errTask = $process.StandardError.ReadToEndAsync()
    $finished = $process.WaitForExit(10000)
    if (-not $finished) { $process.Kill(); $process.WaitForExit() }
    $stdout = $outTask.GetAwaiter().GetResult()
    $stderr = $errTask.GetAwaiter().GetResult()
    $code = $process.ExitCode
    $ok = $finished -and (($expectSuccess -and $code -eq 0) -or (-not $expectSuccess -and $code -ne 0))
    $record = [pscustomobject]@{cwd=$script:cwd; command=$command; stdout=$stdout; stderr=$stderr; exitCode=$code; timedOut=(-not $finished); expectedSuccess=$expectSuccess; matchedExpectation=$ok}
    $script:current.commands.Add($record)
    $checks.Add([pscustomobject]@{command=$command; passed=$ok})
    Write-Host ('> ' + $command)
    Write-Host ($stdout + $stderr).TrimEnd()
    Write-Host ('[exit status ' + $code + ']')
    return $record
}
function Verify([string]$name, [bool]$passed) { $checks.Add([pscustomobject]@{command=$name;passed=$passed}); if (-not $passed) { throw "Verification failed: $name" } }
function Go([string]$folder) { $script:cwd = Join-Path $work $folder; New-Item -ItemType Directory -Path $script:cwd | Out-Null }
try {
    New-Item -ItemType Directory -Path $work, $cross | Out-Null
    Section '00_environment' 'Windows environment and NTFS volumes'
    $null = Run 'ver'
    $null = Run 'echo %DATE% %TIME%'
    $null = Run 'fsutil fsinfo volumeinfo C:'
    $null = Run ('fsutil fsinfo volumeinfo ' + $SecondDrive)

    Section '01_lab1_basic_links' '1.1 Hard and symbolic links: delete and recreate source'
    $null = Run 'echo original data> source.txt'
    $null = Run 'mklink /H hard.txt source.txt'
    $null = Run 'mklink soft.txt source.txt'
    $null = Run 'dir /a:l /b'
    $null = Run 'fsutil hardlink list hard.txt'
    $null = Run 'type hard.txt'
    $null = Run 'type soft.txt'
    $null = Run 'del source.txt'
    $r = Run 'type hard.txt'; Verify 'Hard link retains original data' ($r.stdout.Trim() -eq 'original data')
    $null = Run 'type soft.txt' $false
    $null = Run 'echo new source> source.txt'
    $r = Run 'type soft.txt'; Verify 'Symbolic link resolves recreated source' ($r.stdout.Trim() -eq 'new source')
    $r = Run 'type hard.txt'; Verify 'Hard link still names old file' ($r.stdout.Trim() -eq 'original data')

    Section '02a_lab1_symbolic_delete_source' '1.2a Symbolic chain: delete source'
    Go 'sym-a'
    $null = Run 'echo level 0 data> source.txt'
    $null = Run 'mklink level1 source.txt'
    $null = Run 'mklink level2 level1'
    $null = Run 'dir /a:l'
    $null = Run 'type level2'
    $null = Run 'del source.txt'
    $null = Run 'type level1' $false
    $null = Run 'type level2' $false

    Section '02b_lab1_symbolic_delete_level1' '1.2b Symbolic chain: delete level 1'
    Go 'sym-b'
    $null = Run 'echo level 0 data> source.txt'
    $null = Run 'mklink level1 source.txt'
    $null = Run 'mklink level2 level1'
    $null = Run 'type level2'
    $null = Run 'del level1'
    $null = Run 'dir /a:l'
    $r = Run 'type source.txt'; Verify 'Deleting symbolic level 1 preserves source' ($r.stdout.Trim() -eq 'level 0 data')
    $null = Run 'type level2' $false

    Section '02c_lab1_hard_link_chains' '1.2c-d Hard links: delete source or level 1'
    Go 'hard-a'
    $null = Run 'echo level 0 data> source.txt'
    $null = Run 'mklink /H level1 source.txt'
    $null = Run 'mklink /H level2 level1'
    $null = Run 'fsutil hardlink list level2'
    $null = Run 'del source.txt'
    $null = Run 'fsutil hardlink list level2'
    $null = Run 'type level1'
    $r = Run 'type level2'; Verify 'Hard level 2 survives source deletion' ($r.stdout.Trim() -eq 'level 0 data')
    Go 'hard-b'
    $null = Run 'echo level 0 data> source.txt'
    $null = Run 'mklink /H level1 source.txt'
    $null = Run 'mklink /H level2 level1'
    $null = Run 'del level1'
    $null = Run 'fsutil hardlink list level2'
    $r = Run 'type level2'; Verify 'Hard level 2 survives level 1 deletion' ($r.stdout.Trim() -eq 'level 0 data')

    Section '03a_lab1_directory_links' '1.3 Directory hard link, symbolic link, and junction'
    $script:cwd = $work
    $null = Run 'mkdir top\sub'
    $null = Run 'echo file in sub> top\sub\file.txt'
    $null = Run 'mklink /H top-hard top' $false
    $null = Run 'mklink /D top-soft top'
    $null = Run ('mklink /J top-junction "' + $work + '\top"'); $junctions.Add((Join-Path $work 'top-junction'))
    $null = Run 'dir /a:l'
    $null = Run 'type top-soft\sub\file.txt'
    $null = Run 'type top-junction\sub\file.txt'

    Section '03b_lab1_directory_loop' '1.3 Loop to ancestor and recursive dir /s'
    $null = Run ('mklink /J top\sub\up "' + $work + '\top"'); $junctions.Add((Join-Path $work 'top\sub\up'))
    $null = Run 'dir /a:l top\sub'
    $null = Run 'type top\sub\up\sub\up\sub\file.txt'
    $null = Run 'dir /s /b top > loop-list.txt'
    $null = Run 'find /c /v "" loop-list.txt'
    $null = Run 'powershell.exe -NoProfile -Command "$p=Get-Content -LiteralPath loop-list.txt; ''First 3 paths:''; $p | Select-Object -First 3; ''Last 3 paths:''; $p | Select-Object -Last 3; ''Maximum path characters:''; ($p | Measure-Object -Property Length -Maximum).Maximum"'
    $null = Run 'rmdir top\sub\up'

    Section '04_lab1_cross_drive' '1.4 Link on D: to source on C:'
    $null = Run ('mklink /H "' + $cross + '\hard.txt" "' + $work + '\hard.txt"') $false
    $null = Run ('mklink "' + $cross + '\soft.txt" "' + $work + '\hard.txt"')
    $null = Run ('mklink /J "' + $cross + '\junction" "' + $work + '\top"'); $junctions.Add((Join-Path $cross 'junction'))
    $r = Run ('type "' + $cross + '\soft.txt"'); Verify 'Cross-drive symbolic link reads C: data' ($r.stdout.Trim() -eq 'original data')
    $r = Run ('type "' + $cross + '\junction\sub\file.txt"'); Verify 'Cross-drive junction reads C: directory' ($r.stdout.Trim() -eq 'file in sub')
    $null = Run ('dir /a:l "' + $cross + '"')
} finally {
    $all = [Collections.Generic.List[string]]::new()
    foreach ($section in $sections) {
        $lines = [Collections.Generic.List[string]]::new()
        $lines.Add('### ' + $section.title)
        foreach ($command in $section.commands) {
            $lines.Add(''); $lines.Add($command.cwd + '> ' + $command.command)
            if ($command.stdout.TrimEnd()) { $lines.Add($command.stdout.TrimEnd()) }
            if ($command.stderr.TrimEnd()) { $lines.Add($command.stderr.TrimEnd()) }
            $lines.Add('[exit status ' + $command.exitCode + ']')
        }
        $all.AddRange($lines)
    }
    $all | Set-Content -LiteralPath (Join-Path $results '05_windows_links.txt') -Encoding UTF8
    # Remove reparse points explicitly before recursively removing owned directories.
    foreach ($link in $junctions) { if ([IO.Directory]::Exists($link)) { [IO.Directory]::Delete($link) } }
    $dirSoft = Join-Path $work 'top-soft'
    if ([IO.Directory]::Exists($dirSoft)) { [IO.Directory]::Delete($dirSoft) }
    foreach ($path in @($cross, $work)) {
        $full = [IO.Path]::GetFullPath($path)
        if ([IO.Path]::GetFileName($full) -ne $runId) { throw 'Cleanup path validation failed.' }
        if (Test-Path -LiteralPath $full) { Remove-Item -LiteralPath $full -Recurse -Force }
    }
}
if (@($checks | Where-Object { -not $_.passed }).Count) { throw 'Unexpected result. Inspect results/05_windows_links.txt.' }
Write-Host ('Completed ' + $checks.Count + ' command and content checks.')
