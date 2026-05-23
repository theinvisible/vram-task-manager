<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE" sourcelanguage="en_US">
<context>
    <name>MainWindow</name>
    <message>
        <source>VRAM Task Manager</source>
        <translation>VRAM Task Manager</translation>
    </message>
    <message>
        <source>Processes</source>
        <translation>Prozesse</translation>
    </message>
    <message>
        <source>GPU %1 · %2</source>
        <translation>GPU %1 · %2</translation>
    </message>
    <message>
        <source>Filter processes…</source>
        <translation>Prozesse filtern…</translation>
    </message>
    <message>
        <source>Error: %1</source>
        <translation>Fehler: %1</translation>
    </message>
    <message>
        <source>n/a</source>
        <translation>n. v.</translation>
    </message>
    <message>
        <source>Update available</source>
        <translation>Update verfügbar</translation>
    </message>
    <message>
        <source>&lt;b&gt;VRAM Task Manager %1&lt;/b&gt; is available — you have %2.</source>
        <translation>&lt;b&gt;VRAM Task Manager %1&lt;/b&gt; ist verfügbar — installiert ist %2.</translation>
    </message>
    <message>
        <source>Download and install the new version now?</source>
        <translation>Die neue Version jetzt herunterladen und installieren?</translation>
    </message>
    <message>
        <source>Update now</source>
        <translation>Jetzt aktualisieren</translation>
    </message>
    <message>
        <source>Remind me later</source>
        <translation>Später erinnern</translation>
    </message>
    <message>
        <source>Skip this version</source>
        <translation>Diese Version überspringen</translation>
    </message>
    <message>
        <source>Release notes…</source>
        <translation>Release-Notes…</translation>
    </message>
    <message>
        <source>Downloading update…</source>
        <translation>Update wird heruntergeladen…</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <source>Update failed</source>
        <translation>Update fehlgeschlagen</translation>
    </message>
    <message>
        <source>Could not download the installer:
%1</source>
        <translation>Installer konnte nicht heruntergeladen werden:
%1</translation>
    </message>
    <message>
        <source>Could not start the installer:
%1</source>
        <translation>Installer konnte nicht gestartet werden:
%1</translation>
    </message>
    <message>
        <source>About VRAM Task Manager</source>
        <translation>Über VRAM Task Manager</translation>
    </message>
</context>
<context>
    <name>AboutDialog</name>
    <message>
        <source>About VRAM Task Manager</source>
        <translation>Über VRAM Task Manager</translation>
    </message>
    <message>
        <source>Version %1</source>
        <translation>Version %1</translation>
    </message>
    <message>
        <source>Live per-process VRAM usage for Windows.</source>
        <translation>Live-VRAM-Nutzung pro Prozess unter Windows.</translation>
    </message>
    <message>
        <source>© 2026 Rene Hadler
Built with Qt %1</source>
        <translation>© 2026 Rene Hadler
Erstellt mit Qt %1</translation>
    </message>
    <message>
        <source>View project on GitHub</source>
        <translation>Projekt auf GitHub ansehen</translation>
    </message>
    <message>
        <source>Install</source>
        <translation>Installieren</translation>
    </message>
    <message>
        <source>Check again</source>
        <translation>Erneut prüfen</translation>
    </message>
    <message>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <source>Update check pending.</source>
        <translation>Update-Prüfung steht aus.</translation>
    </message>
    <message>
        <source>Checking for updates…</source>
        <translation>Suche nach Updates…</translation>
    </message>
    <message>
        <source>You're up to date — version %1 is the latest release.</source>
        <translation>Du bist aktuell — Version %1 ist die neueste Veröffentlichung.</translation>
    </message>
    <message>
        <source>Update available: &lt;b&gt;%1&lt;/b&gt; (you have %2).</source>
        <translation>Update verfügbar: &lt;b&gt;%1&lt;/b&gt; (installiert ist %2).</translation>
    </message>
    <message>
        <source>Update check unavailable — check your internet connection.</source>
        <translation>Update-Prüfung nicht möglich — bitte Internetverbindung prüfen.</translation>
    </message>
</context>
<context>
    <name>VramModel</name>
    <message>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <source>Process</source>
        <translation>Prozess</translation>
    </message>
    <message>
        <source>GPU</source>
        <translation>GPU</translation>
    </message>
    <message>
        <source>Dedicated (VRAM)</source>
        <translation>Dediziert (VRAM)</translation>
    </message>
    <message>
        <source>Shared (system RAM)</source>
        <translation>Geteilt (System-RAM)</translation>
    </message>
    <message>
        <source>Total (commit)</source>
        <translation>Gesamt (Commit)</translation>
    </message>
    <message>
        <source>NVIDIA (resident)</source>
        <translation>NVIDIA (resident)</translation>
    </message>
    <message>
        <source>GPU %1</source>
        <translation>GPU %1</translation>
    </message>
    <message>
        <source>Process ID (PID) of the Windows process that holds GPU memory.</source>
        <translation>Prozess-ID (PID) des Windows-Prozesses, der GPU-Speicher belegt.</translation>
    </message>
    <message>
        <source>Name of the process's executable.</source>
        <translation>Name der ausführbaren Datei des Prozesses.</translation>
    </message>
    <message>
        <source>GPU(s) on which the process holds memory — the index matches
the GPU cards shown above. Multiple values mean the process
is running on several adapters at the same time.</source>
        <translation>GPU(s), auf denen der Prozess Speicher belegt — der Index entspricht
den oben gezeigten GPU-Karten. Mehrere Werte bedeuten, dass der
Prozess auf mehreren Adaptern gleichzeitig läuft.</translation>
    </message>
    <message>
        <source>Dedicated graphics memory (VRAM) that the process holds on the
GPU itself. Source: Windows performance counter (PDH,
GPU Process Memory → Dedicated Usage). Summed across all GPUs.</source>
        <translation>Dedizierter Grafikspeicher (VRAM), den der Prozess auf der GPU
selbst belegt. Quelle: Windows-Performance-Counter (PDH,
GPU Process Memory → Dedicated Usage). Summiert über alle GPUs.</translation>
    </message>
    <message>
        <source>Shared memory: portion of system RAM made available to the GPU
(used heavily by iGPUs; on dGPUs typically only as overflow).
Source: PDH, GPU Process Memory → Shared Usage.</source>
        <translation>Geteilter Speicher: Teil des System-RAMs, der der GPU zur
Verfügung gestellt wird (von iGPUs intensiv genutzt, bei
dGPUs üblicherweise nur als Überlauf). Quelle: PDH,
GPU Process Memory → Shared Usage.</translation>
    </message>
    <message>
        <source>Total GPU memory the process has committed (Dedicated + Shared).
Matches what Windows reports as the process's overall GPU
memory usage.</source>
        <translation>Gesamter GPU-Speicher, den der Prozess committed hat
(Dediziert + Geteilt). Entspricht dem, was Windows als gesamten
GPU-Speicherverbrauch des Prozesses meldet.</translation>
    </message>
    <message>
        <source>Resident VRAM as reported by NVIDIA NVML — only available for
NVIDIA cards. Usually the most accurate figure for data actually
resident on the GPU. &quot;—&quot; means NVML has no data for this
process (e.g. non-NVIDIA process).</source>
        <translation>Resident VRAM laut NVIDIA NVML — nur für NVIDIA-Karten verfügbar.
In der Regel der genaueste Wert für tatsächlich auf der GPU
liegende Daten. „—&quot; bedeutet, dass NVML keine Daten zu diesem
Prozess liefert (z. B. nicht-NVIDIA-Prozess).</translation>
    </message>
</context>
<context>
    <name>VramSampler</name>
    <message>
        <source>Performance counter &quot;GPU Process Memory&quot; not available. Requires Windows 10 1709+ with a WDDM 2.0 driver.</source>
        <translation>Performance-Counter „GPU Process Memory" nicht verfügbar. Benötigt Windows 10 1709+ mit WDDM-2.0-Treiber.</translation>
    </message>
</context>
<context>
    <name>UpdateChecker</name>
    <message>
        <source>No installer asset available for this release.</source>
        <translation>Kein Installer-Asset für dieses Release vorhanden.</translation>
    </message>
    <message>
        <source>Cannot write to %1: %2</source>
        <translation>Schreiben nach %1 fehlgeschlagen: %2</translation>
    </message>
    <message>
        <source>Download cancelled.</source>
        <translation>Download abgebrochen.</translation>
    </message>
</context>
<context>
    <name>NvmlSampler</name>
    <message>
        <source>nvml.dll not found — no NVIDIA driver?</source>
        <translation>nvml.dll nicht gefunden — kein NVIDIA-Treiber?</translation>
    </message>
    <message>
        <source>nvml.dll: required symbols missing</source>
        <translation>nvml.dll: erforderliche Symbole fehlen</translation>
    </message>
    <message>
        <source>NVML: *RunningProcesses_v3 missing</source>
        <translation>NVML: *RunningProcesses_v3 fehlt</translation>
    </message>
    <message>
        <source>Per-process VRAM not available (WDDM mode, consumer GPU). Per-card total above is reliable.</source>
        <translation>Per-Prozess-VRAM nicht verfügbar (WDDM-Modus, Consumer-GPU). Karten-Gesamtwert oben ist verlässlich.</translation>
    </message>
</context>
</TS>
