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
