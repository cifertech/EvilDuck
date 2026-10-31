# Copy scripts/ -> ../data/ for ESP32 Sketch Data Upload (SPIFFS)
$scriptsDir = $PSScriptRoot
$dataDir    = Join-Path (Split-Path $scriptsDir -Parent) "data"
$count = 0
Get-ChildItem -Path $scriptsDir -Recurse -Include *.txt,*.duck | ForEach-Object {
    if ($_.Name -eq "README.txt") { return }
    $rel = $_.FullName.Substring($scriptsDir.Length + 1)
    $out = Join-Path $dataDir $rel
    $dir = Split-Path $out -Parent
    if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
    Copy-Item -Force $_.FullName $out
    $count++
}
Write-Host "Copied $count script(s) to $dataDir"
Write-Host "Flash SPIFFS: Arduino IDE -> Tools -> ESP32 Sketch Data Upload"
Write-Host "Run on device: demos/01_hello_windows.txt (path as shown in /list)"
