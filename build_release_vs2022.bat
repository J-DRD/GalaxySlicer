set WP=%CD%
cd deps
mkdir build
cd build
set DEPS=%CD%/GalaxySlicer_dep

if "%1"=="slicer" (
    GOTO :slicer
)

echo "building deps.."
cmake ../ -G "Visual Studio 17 2022" -A x64 -DDESTDIR="%CD%/GalaxySlicer_dep" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target deps -- -m

echo "downloading Python.."
set PY_URL=https://www.python.org/ftp/python/3.12.0/python-3.12.0-embed-amd64.zip
set PY=%WP%/deps/build/GalaxySlicer_dep

cd %PY%
mkdir python
cd python

set PY_DIR=%CD%

curl -o %PY_DIR%\python_embed.zip %PY_URL%

powershell -command "Expand-Archive -Path %PY_DIR%\python_embed.zip -DestinationPath %PY_DIR%"

del %PY_DIR%\python_embed.zip

if "%1"=="deps" exit /b 0
goto :slicer
goto :bblprofiledir
bblprofiledir:
echo "Locating profiles/BBL"
setlocal enabledelayedexpansion
set "targetDirectory="
set "parentDirectory=%WP%"
for /f %%I in ('dir /b /ad "%parentDirectory%"') do (
    set "currentDirectory=%%I"
    set "currentDirectory=!currentDirectory: =!"
    echo "!currentDirectory!" | find /i "profiles/BBL" >nul && (
        set "targetDirectory=!currentDirectory!"
        goto :foundbblprofilesdir
    )
:foundbblprofilesdir
if defined targetDirectory (
    echo Target directory found: %targetDirectory%
    echo "cloning latest profiles for X1-Carbon from sources"
    gh repo clone shyblower/Shyblowers-OrcaSlicer-Profiles
    powershell -command "Copy-Item -Path 'Shyblowers-OrcaSlicer-Profiles/*' -Destination %targetDirectory%/ -Recurse"
m
) else (
    echo Target directory not found.
)

:slicer
echo "building GalaxySlicer..."
cd %WP%
mkdir %build_dir%
cd %build_dir%

cmake .. -G "Visual Studio 17 2022" -A x64 -DBBL_RELEASE_TO_PUBLIC=1 -DCMAKE_PREFIX_PATH="%DEPS%/usr/local" -DCMAKE_INSTALL_PREFIX="./GalaxySlicer" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release --target ALL_BUILD -- -m
cd ..
call run_gettext.bat
cd %build_dir%
cmake --build . --target install --config %build_type%

echo "copying Python..."

cd %WP%/build/GalaxySlicer
mkdir python
cd python

set PY_DEST=%CD%
set PY_DEPS=%WP%\deps\build\GalaxySlicer_dep\python

powershell -command "Copy-Item -Path %PY_DEPS%\* -Destination %PY_DEST% -Recurse"

echo "create System folders..."

cd %WP%/build/GalaxySlicer
mkdir applications

echo "building complete..."
