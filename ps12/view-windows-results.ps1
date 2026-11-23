# Compile the read-only viewer with the .NET Framework included with Windows.
$ErrorActionPreference = 'Stop'
$viewerDir = Join-Path $env:TEMP 'PS12-WindowsResultsViewer'
New-Item -ItemType Directory -Force -Path $viewerDir | Out-Null
# Derive the viewer data from the single committed transcript. The JSON is
# temporary viewer input, not a second set of evidence stored in the repository.
$transcript = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'results\05_windows_links.txt') -Raw
$headings = [regex]::Matches($transcript, '(?m)^### (.+)\r?$')
$sections = @()
for ($i=0; $i -lt $headings.Count; $i++) {
    $start = $headings[$i].Index + $headings[$i].Length
    $end = if ($i+1 -lt $headings.Count) { $headings[$i+1].Index } else { $transcript.Length }
    $body = $transcript.Substring($start, $end-$start)
    $prompts = [regex]::Matches($body, '(?m)^([A-Za-z]:\\[^\r\n]*?)> ([^\r\n]*)\r?$')
    $commands = @()
    for ($j=0; $j -lt $prompts.Count; $j++) {
        $outputStart = $prompts[$j].Index + $prompts[$j].Length
        $outputEnd = if ($j+1 -lt $prompts.Count) { $prompts[$j+1].Index } else { $body.Length }
        $output = $body.Substring($outputStart, $outputEnd-$outputStart).TrimEnd([char[]]"`r`n")
        if ($output.StartsWith("`r`n")) { $output = $output.Substring(2) }
        elseif ($output.StartsWith("`n")) { $output = $output.Substring(1) }
        $status = [regex]::Match($output, '(?s)^(.*?)\r?\n?\[exit status (-?\d+)\]\s*$')
        if (-not $status.Success) { throw 'Missing exit status in Windows transcript.' }
        $commands += [pscustomobject]@{cwd=$prompts[$j].Groups[1].Value; command=$prompts[$j].Groups[2].Value; stdout=$status.Groups[1].Value; stderr=''; exitCode=[int]$status.Groups[2].Value}
    }
    if ($commands.Count) { $sections += [pscustomobject]@{id=$i; title=$headings[$i].Groups[1].Value.TrimEnd(); commands=$commands} }
}
if (-not $sections.Count) { throw 'No command results found in Windows transcript.' }
$manifest = Join-Path $viewerDir 'manifest.json'
[pscustomobject]@{capturedAt='See the recorded date in the Environment tab'; sections=$sections} | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifest -Encoding UTF8
$viewer = Join-Path $viewerDir 'WindowsResultsViewer.exe'
$compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
& $compiler /nologo /target:winexe /r:System.Windows.Forms.dll /r:System.Drawing.dll /r:System.Web.Extensions.dll "/out:$viewer" (Join-Path $PSScriptRoot 'WindowsResultsViewer.cs')
if ($LASTEXITCODE -ne 0) { throw 'Could not compile the results viewer.' }
Start-Process -FilePath $viewer -ArgumentList ('"' + $manifest + '"') -WindowStyle Normal
