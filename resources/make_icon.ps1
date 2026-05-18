Add-Type -AssemblyName System.Drawing

function New-IconBitmap([int]$size) {
    $bmp = New-Object System.Drawing.Bitmap $size, $size, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode     = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $g.Clear([System.Drawing.Color]::Transparent)

    $radius = [Math]::Max(2, [int]($size * 0.20))
    $d = $radius * 2

    # Background: rounded square with diagonal gradient
    $bgPath = New-Object System.Drawing.Drawing2D.GraphicsPath
    $bgPath.AddArc(0, 0, $d, $d, 180, 90)
    $bgPath.AddArc($size - $d, 0, $d, $d, 270, 90)
    $bgPath.AddArc($size - $d, $size - $d, $d, $d, 0, 90)
    $bgPath.AddArc(0, $size - $d, $d, $d, 90, 90)
    $bgPath.CloseFigure()

    $bgBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush `
        (New-Object System.Drawing.Point 0, 0), `
        (New-Object System.Drawing.Point $size, $size), `
        ([System.Drawing.Color]::FromArgb(255, 36, 41, 51)), `
        ([System.Drawing.Color]::FromArgb(255, 18, 21, 27))
    $g.FillPath($bgBrush, $bgPath)
    $bgBrush.Dispose()

    # Thin inner stroke for definition
    if ($size -ge 32) {
        $strokePen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 50, 55, 66)), ([float]([Math]::Max(1, $size / 128.0)))
        $g.DrawPath($strokePen, $bgPath)
        $strokePen.Dispose()
    }
    $bgPath.Dispose()

    # Four ascending bars
    $padding   = [int]($size * 0.22)
    $barAreaW  = $size - 2 * $padding
    $barAreaH  = $size - 2 * $padding
    $barCount  = 4
    $gapRatio  = 0.22
    $barW      = [int]($barAreaW / ($barCount + ($barCount - 1) * $gapRatio))
    $gap       = [int]($barW * $gapRatio)
    $heights   = @(0.32, 0.55, 0.78, 1.00)
    $barRadius = [Math]::Max(1, [int]($barW * 0.22))

    $accent  = [System.Drawing.Color]::FromArgb(255, 58, 123, 213)
    $accent2 = [System.Drawing.Color]::FromArgb(255, 96, 165, 250)

    $x = $padding
    for ($i = 0; $i -lt $barCount; $i++) {
        $h = [int]($barAreaH * $heights[$i])
        $y = $size - $padding - $h
        $br = [Math]::Min($barRadius, [int]($h / 2))
        $bd = $br * 2

        $barPath = New-Object System.Drawing.Drawing2D.GraphicsPath
        $barPath.AddArc($x, $y, $bd, $bd, 180, 90)
        $barPath.AddArc($x + $barW - $bd, $y, $bd, $bd, 270, 90)
        $barPath.AddLine($x + $barW, $y + $br, $x + $barW, $y + $h)
        $barPath.AddLine($x + $barW, $y + $h, $x, $y + $h)
        $barPath.AddLine($x, $y + $h, $x, $y + $br)
        $barPath.CloseFigure()

        $barBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush `
            (New-Object System.Drawing.Point $x, $y), `
            (New-Object System.Drawing.Point $x, ($y + $h)), `
            $accent2, $accent
        $g.FillPath($barBrush, $barPath)
        $barBrush.Dispose()
        $barPath.Dispose()

        $x += $barW + $gap
    }

    $g.Dispose()
    return $bmp
}

$sizes = @(16, 24, 32, 48, 64, 128, 256)
$pngBuffers = @{}
foreach ($s in $sizes) {
    $bmp = New-IconBitmap $s
    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $pngBuffers[$s] = $ms.ToArray()
    $ms.Dispose()
    $bmp.Dispose()
}

# Build ICO container with PNG payloads
$icoStream = New-Object System.IO.MemoryStream
$writer = New-Object System.IO.BinaryWriter $icoStream

$writer.Write([UInt16]0)
$writer.Write([UInt16]1)
$writer.Write([UInt16]$sizes.Count)

$headerSize = 6 + 16 * $sizes.Count
$offset = $headerSize
foreach ($s in $sizes) {
    $bytes = $pngBuffers[$s]
    $w = if ($s -ge 256) { [byte]0 } else { [byte]$s }
    $writer.Write([byte]$w)
    $writer.Write([byte]$w)
    $writer.Write([byte]0)
    $writer.Write([byte]0)
    $writer.Write([UInt16]1)
    $writer.Write([UInt16]32)
    $writer.Write([UInt32]$bytes.Length)
    $writer.Write([UInt32]$offset)
    $offset += $bytes.Length
}
foreach ($s in $sizes) {
    $writer.Write($pngBuffers[$s])
}

$icoBytes = $icoStream.ToArray()
$writer.Dispose()
$icoStream.Dispose()

$out = Join-Path $PSScriptRoot "app.ico"
[System.IO.File]::WriteAllBytes($out, $icoBytes)
Write-Output "Wrote $out ($($icoBytes.Length) bytes, $($sizes.Count) sizes)"
