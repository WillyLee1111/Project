$content = Get-Content -Encoding UTF8 "data/dictionary.txt"
$newLines = @()
foreach ($line in $content) {
    if ($line.Trim() -eq "") { continue }
    $parts = $line -split '\|'
    if ($parts.Length -eq 4) {
        $w = $parts[0].ToLower()
        $m = $parts[1]
        $p = $parts[2]
        $t = $parts[3].ToLower()
        $newLines += "$w|$m|$p|$t"
    } else {
        $newLines += $line
    }
}
$newLines | Set-Content -Encoding UTF8 "data/dictionary.txt"
