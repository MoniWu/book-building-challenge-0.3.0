@echo off
echo Building book-building project...

REM Check if compiler is available
where cl.exe >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo MSVC compiler not found. Trying to setup Visual Studio environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

REM Create bin directory
if not exist bin mkdir bin

REM Compile all cpp files
echo Compiling source files...
cl.exe /EHsc /std:c++20 /O2 /W4 /Icpp /Fo:cpp\ cpp\main.cpp cpp\pcap.cpp cpp\protocol.cpp cpp\book.cpp cpp\snapshot_writer.cpp /Fe:bin\build-book.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running the program...
    bin\build-book.exe --input input.pcap --output output.csv

    if %ERRORLEVEL% EQU 0 (
        echo Program completed successfully!
        echo Running comparison metrics...
        python compare_metrics.py
    )
) else (
    echo Build failed!
)
