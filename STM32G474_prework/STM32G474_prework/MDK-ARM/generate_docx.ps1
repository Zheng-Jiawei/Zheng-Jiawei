$ErrorActionPreference = "Stop"

$sourcePath = Join-Path $PSScriptRoot "CODE_OVERVIEW.md"
$outputPath = Join-Path $PSScriptRoot "STM32_G474_Project_Code_Overview.docx"
$buildRoot = Join-Path $PSScriptRoot ".docx_build"

function Escape-XmlText {
    param([string]$Text)

    if ($null -eq $Text) {
        return ""
    }

    return [System.Security.SecurityElement]::Escape($Text)
}

function New-RunXml {
    param(
        [string]$Text,
        [switch]$Code
    )

    $escaped = Escape-XmlText $Text
    $spaceAttr = ""

    if ($Text -match "^\s" -or $Text -match "\s$" -or $Text -match "\s{2,}") {
        $spaceAttr = ' xml:space="preserve"'
    }

    if ($Code) {
        return "<w:r><w:rPr><w:rStyle w:val=`"CodeInline`"/></w:rPr><w:t$spaceAttr>$escaped</w:t></w:r>"
    }

    return "<w:r><w:t$spaceAttr>$escaped</w:t></w:r>"
}

function Convert-InlineToRuns {
    param([string]$Text)

    if ([string]::IsNullOrEmpty($Text)) {
        return "<w:r/>"
    }

    $parts = $Text -split "`u0060", -1
    $runs = New-Object System.Collections.Generic.List[string]

    for ($i = 0; $i -lt $parts.Count; $i++) {
        $segment = $parts[$i]

        if ($segment.Length -eq 0) {
            continue
        }

        if (($i % 2) -eq 1) {
            $runs.Add((New-RunXml -Text $segment -Code))
        }
        else {
            $runs.Add((New-RunXml -Text $segment))
        }
    }

    if ($runs.Count -eq 0) {
        return "<w:r/>"
    }

    return ($runs -join "")
}

function New-ParagraphXml {
    param(
        [string]$Text,
        [string]$Style = "",
        [int]$NumId = 0,
        [int]$Level = 0,
        [int]$LeftIndent = 0,
        [switch]$PageBreakBefore
    )

    $pPrParts = New-Object System.Collections.Generic.List[string]

    if ($Style) {
        $pPrParts.Add("<w:pStyle w:val=`"$Style`"/>")
    }

    if ($NumId -gt 0) {
        $pPrParts.Add("<w:numPr><w:ilvl w:val=`"$Level`"/><w:numId w:val=`"$NumId`"/></w:numPr>")
    }

    if ($LeftIndent -gt 0 -and $NumId -eq 0) {
        $pPrParts.Add("<w:ind w:left=`"$LeftIndent`"/>")
    }

    if ($PageBreakBefore) {
        $pPrParts.Add("<w:pageBreakBefore/>")
    }

    $pPr = ""
    if ($pPrParts.Count -gt 0) {
        $pPr = "<w:pPr>$($pPrParts -join '')</w:pPr>"
    }

    $runs = Convert-InlineToRuns $Text
    return "<w:p>$pPr$runs</w:p>"
}

function New-BlankParagraphXml {
    return "<w:p><w:r/></w:p>"
}

$lines = Get-Content $sourcePath -Encoding UTF8
$title = "STM32 G474 Project Code Overview"
$subtitle = "Generated from CODE_OVERVIEW.md on $(Get-Date -Format 'yyyy-MM-dd HH:mm')"
$body = New-Object System.Collections.Generic.List[string]

$body.Add((New-ParagraphXml -Text $title -Style "Title"))
$body.Add((New-ParagraphXml -Text $subtitle -Style "Subtitle"))
$body.Add((New-BlankParagraphXml))

foreach ($line in $lines) {
    if ([string]::IsNullOrWhiteSpace($line)) {
        $body.Add((New-BlankParagraphXml))
        continue
    }

    if ($line -match '^#\s+(.+)$') {
        continue
    }

    if ($line -match '^##\s+(.+)$') {
        $body.Add((New-ParagraphXml -Text $Matches[1] -Style "Heading1"))
        continue
    }

    if ($line -match '^###\s+(.+)$') {
        $body.Add((New-ParagraphXml -Text $Matches[1] -Style "Heading2"))
        continue
    }

    if ($line -match '^####\s+(.+)$') {
        $body.Add((New-ParagraphXml -Text $Matches[1] -Style "Heading3"))
        continue
    }

    if ($line -match '^(\s*)-\s+(.+)$') {
        $indentChars = $Matches[1].Length
        $level = [Math]::Min([int]($indentChars / 2), 3)
        $body.Add((New-ParagraphXml -Text $Matches[2] -Style "ListParagraph" -NumId 1 -Level $level))
        continue
    }

    if ($line -match '^(\s*)\d+\.\s+(.+)$') {
        $indentChars = $Matches[1].Length
        $level = [Math]::Min([int]($indentChars / 2), 3)
        $body.Add((New-ParagraphXml -Text $Matches[2] -Style "ListParagraph" -NumId 2 -Level $level))
        continue
    }

    if ($line -match '^(\s+)(.+)$') {
        $indent = [Math]::Min($Matches[1].Length * 180, 1440)
        $body.Add((New-ParagraphXml -Text $Matches[2] -LeftIndent $indent))
        continue
    }

    $body.Add((New-ParagraphXml -Text $line))
}

$documentXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:wpc="http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas"
 xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006"
 xmlns:o="urn:schemas-microsoft-com:office:office"
 xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
 xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"
 xmlns:v="urn:schemas-microsoft-com:vml"
 xmlns:wp14="http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing"
 xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
 xmlns:w10="urn:schemas-microsoft-com:office:word"
 xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
 xmlns:w14="http://schemas.microsoft.com/office/word/2010/wordml"
 xmlns:wpg="http://schemas.microsoft.com/office/word/2010/wordprocessingGroup"
 xmlns:wpi="http://schemas.microsoft.com/office/word/2010/wordprocessingInk"
 xmlns:wne="http://schemas.microsoft.com/office/word/2006/wordml"
 xmlns:wps="http://schemas.microsoft.com/office/word/2010/wordprocessingShape"
 mc:Ignorable="w14 wp14">
  <w:body>
$($body -join "`r`n")
    <w:sectPr>
      <w:pgSz w:w="11906" w:h="16838"/>
      <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440" w:header="720" w:footer="720" w:gutter="0"/>
      <w:cols w:space="720"/>
      <w:docGrid w:linePitch="360"/>
    </w:sectPr>
  </w:body>
</w:document>
"@

$stylesXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
  <w:docDefaults>
    <w:rPrDefault>
      <w:rPr>
        <w:rFonts w:ascii="Calibri" w:hAnsi="Calibri" w:eastAsia="Microsoft YaHei" w:cs="Calibri"/>
        <w:sz w:val="22"/>
        <w:szCs w:val="22"/>
        <w:lang w:val="en-US" w:eastAsia="zh-CN"/>
      </w:rPr>
    </w:rPrDefault>
    <w:pPrDefault>
      <w:pPr>
        <w:spacing w:after="120" w:line="360" w:lineRule="auto"/>
      </w:pPr>
    </w:pPrDefault>
  </w:docDefaults>
  <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
    <w:name w:val="Normal"/>
    <w:qFormat/>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Title">
    <w:name w:val="Title"/>
    <w:basedOn w:val="Normal"/>
    <w:next w:val="Normal"/>
    <w:qFormat/>
    <w:pPr>
      <w:jc w:val="center"/>
      <w:spacing w:before="240" w:after="120"/>
    </w:pPr>
    <w:rPr>
      <w:b/>
      <w:color w:val="1F3A5F"/>
      <w:rFonts w:ascii="Calibri" w:hAnsi="Calibri" w:eastAsia="Microsoft YaHei"/>
      <w:sz w:val="32"/>
      <w:szCs w:val="32"/>
    </w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Subtitle">
    <w:name w:val="Subtitle"/>
    <w:basedOn w:val="Normal"/>
    <w:qFormat/>
    <w:pPr>
      <w:jc w:val="center"/>
      <w:spacing w:after="240"/>
    </w:pPr>
    <w:rPr>
      <w:color w:val="666666"/>
      <w:sz w:val="20"/>
      <w:szCs w:val="20"/>
    </w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Heading1">
    <w:name w:val="heading 1"/>
    <w:basedOn w:val="Normal"/>
    <w:next w:val="Normal"/>
    <w:qFormat/>
    <w:pPr>
      <w:spacing w:before="240" w:after="80"/>
      <w:outlineLvl w:val="0"/>
    </w:pPr>
    <w:rPr>
      <w:b/>
      <w:color w:val="1F3A5F"/>
      <w:sz w:val="28"/>
      <w:szCs w:val="28"/>
    </w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Heading2">
    <w:name w:val="heading 2"/>
    <w:basedOn w:val="Normal"/>
    <w:next w:val="Normal"/>
    <w:qFormat/>
    <w:pPr>
      <w:spacing w:before="180" w:after="60"/>
      <w:outlineLvl w:val="1"/>
    </w:pPr>
    <w:rPr>
      <w:b/>
      <w:color w:val="2D5D7B"/>
      <w:sz w:val="24"/>
      <w:szCs w:val="24"/>
    </w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Heading3">
    <w:name w:val="heading 3"/>
    <w:basedOn w:val="Normal"/>
    <w:next w:val="Normal"/>
    <w:qFormat/>
    <w:pPr>
      <w:spacing w:before="120" w:after="40"/>
      <w:outlineLvl w:val="2"/>
    </w:pPr>
    <w:rPr>
      <w:b/>
      <w:color w:val="3E7A92"/>
      <w:sz w:val="22"/>
      <w:szCs w:val="22"/>
    </w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="ListParagraph">
    <w:name w:val="List Paragraph"/>
    <w:basedOn w:val="Normal"/>
    <w:qFormat/>
    <w:pPr>
      <w:spacing w:after="40"/>
    </w:pPr>
  </w:style>
  <w:style w:type="character" w:styleId="CodeInline">
    <w:name w:val="Code Inline"/>
    <w:basedOn w:val="DefaultParagraphFont"/>
    <w:rPr>
      <w:rFonts w:ascii="Consolas" w:hAnsi="Consolas" w:eastAsia="Consolas"/>
      <w:color w:val="B00020"/>
      <w:shd w:val="clear" w:color="auto" w:fill="F3F4F6"/>
      <w:sz w:val="20"/>
      <w:szCs w:val="20"/>
    </w:rPr>
  </w:style>
</w:styles>
"@

$numberingXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:numbering xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
  <w:abstractNum w:abstractNumId="1">
    <w:multiLevelType w:val="multilevel"/>
    <w:lvl w:ilvl="0">
      <w:start w:val="1"/>
      <w:numFmt w:val="bullet"/>
      <w:lvlText w:val="•"/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>
      <w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr>
    </w:lvl>
    <w:lvl w:ilvl="1">
      <w:start w:val="1"/>
      <w:numFmt w:val="bullet"/>
      <w:lvlText w:val="◦"/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="1080" w:hanging="360"/></w:pPr>
      <w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr>
    </w:lvl>
    <w:lvl w:ilvl="2">
      <w:start w:val="1"/>
      <w:numFmt w:val="bullet"/>
      <w:lvlText w:val="▪"/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="1440" w:hanging="360"/></w:pPr>
      <w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr>
    </w:lvl>
    <w:lvl w:ilvl="3">
      <w:start w:val="1"/>
      <w:numFmt w:val="bullet"/>
      <w:lvlText w:val="•"/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="1800" w:hanging="360"/></w:pPr>
      <w:rPr><w:rFonts w:ascii="Symbol" w:hAnsi="Symbol" w:hint="default"/></w:rPr>
    </w:lvl>
  </w:abstractNum>
  <w:abstractNum w:abstractNumId="2">
    <w:multiLevelType w:val="multilevel"/>
    <w:lvl w:ilvl="0">
      <w:start w:val="1"/>
      <w:numFmt w:val="decimal"/>
      <w:lvlText w:val="%1."/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="720" w:hanging="360"/></w:pPr>
    </w:lvl>
    <w:lvl w:ilvl="1">
      <w:start w:val="1"/>
      <w:numFmt w:val="decimal"/>
      <w:lvlText w:val="%1.%2."/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="1080" w:hanging="360"/></w:pPr>
    </w:lvl>
    <w:lvl w:ilvl="2">
      <w:start w:val="1"/>
      <w:numFmt w:val="decimal"/>
      <w:lvlText w:val="%1.%2.%3."/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="1440" w:hanging="360"/></w:pPr>
    </w:lvl>
    <w:lvl w:ilvl="3">
      <w:start w:val="1"/>
      <w:numFmt w:val="decimal"/>
      <w:lvlText w:val="%1.%2.%3.%4."/>
      <w:lvlJc w:val="left"/>
      <w:pPr><w:ind w:left="1800" w:hanging="360"/></w:pPr>
    </w:lvl>
  </w:abstractNum>
  <w:num w:numId="1"><w:abstractNumId w:val="1"/></w:num>
  <w:num w:numId="2"><w:abstractNumId w:val="2"/></w:num>
</w:numbering>
"@

$contentTypesXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
  <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
  <Override PartName="/word/numbering.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml"/>
</Types>
"@

$rootRelsXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
  <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>
"@

$documentRelsXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering" Target="numbering.xml"/>
</Relationships>
"@

$timestamp = (Get-Date).ToUniversalTime().ToString("yyyy-MM-ddTHH:mm:ssZ")

$coreXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
 xmlns:dc="http://purl.org/dc/elements/1.1/"
 xmlns:dcterms="http://purl.org/dc/terms/"
 xmlns:dcmitype="http://purl.org/dc/dcmitype/"
 xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:title>STM32 G474 Project Code Overview</dc:title>
  <dc:subject>Embedded Code Documentation</dc:subject>
  <dc:creator>Codex</dc:creator>
  <cp:lastModifiedBy>Codex</cp:lastModifiedBy>
  <dcterms:created xsi:type="dcterms:W3CDTF">$timestamp</dcterms:created>
  <dcterms:modified xsi:type="dcterms:W3CDTF">$timestamp</dcterms:modified>
</cp:coreProperties>
"@

$appXml = @"
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"
 xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
  <Application>Microsoft Office Word</Application>
  <DocSecurity>0</DocSecurity>
  <ScaleCrop>false</ScaleCrop>
  <Company>OpenAI</Company>
  <LinksUpToDate>false</LinksUpToDate>
  <SharedDoc>false</SharedDoc>
  <HyperlinksChanged>false</HyperlinksChanged>
  <AppVersion>16.0000</AppVersion>
</Properties>
"@

if (Test-Path $buildRoot) {
    Remove-Item $buildRoot -Recurse -Force
}

if (Test-Path $outputPath) {
    Remove-Item $outputPath -Force
}

New-Item -ItemType Directory -Path $buildRoot | Out-Null
New-Item -ItemType Directory -Path (Join-Path $buildRoot "_rels") | Out-Null
New-Item -ItemType Directory -Path (Join-Path $buildRoot "docProps") | Out-Null
New-Item -ItemType Directory -Path (Join-Path $buildRoot "word") | Out-Null
New-Item -ItemType Directory -Path (Join-Path $buildRoot "word\_rels") | Out-Null

$utf8 = New-Object System.Text.UTF8Encoding($false)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "[Content_Types].xml"), $contentTypesXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "_rels\.rels"), $rootRelsXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "docProps\core.xml"), $coreXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "docProps\app.xml"), $appXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "word\document.xml"), $documentXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "word\styles.xml"), $stylesXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "word\numbering.xml"), $numberingXml, $utf8)
[System.IO.File]::WriteAllText((Join-Path $buildRoot "word\_rels\document.xml.rels"), $documentRelsXml, $utf8)

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory($buildRoot, $outputPath)

Remove-Item $buildRoot -Recurse -Force

Write-Output "Created: $outputPath"
