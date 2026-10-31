# Downloads WiFiDuck locale headers into ./locale/
$base = "https://raw.githubusercontent.com/SpacehuhnTech/WiFiDuck/master/atmega_duck"
$dest = Join-Path $PSScriptRoot "locale"
New-Item -ItemType Directory -Force -Path $dest | Out-Null
$files = @(
  "usb_hid_keys.h", "locale_types.h", "locales.h",
  "locale_us.h", "locale_de.h", "locale_gb.h", "locale_es.h", "locale_fr.h",
  "locale_ru.h", "locale_dk.h", "locale_be.h", "locale_pt.h", "locale_it.h",
  "locale_sk.h", "locale_cz.h", "locale_si.h", "locale_bg.h",
  "locale_cafr.h", "locale_chde.h", "locale_chfr.h", "locale_hu.h"
)
foreach ($f in $files) {
  $out = Join-Path $dest $f
  Write-Host "Fetching $f ..."
  curl.exe -fsSL "$base/$f" -o $out
  if ($LASTEXITCODE -ne 0) { Write-Warning "Failed: $f" }
}
Write-Host "Done. Files in $dest"
