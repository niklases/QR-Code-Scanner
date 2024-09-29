powershell -Command "$ProgressPreference = 'SilentlyContinue';Invoke-WebRequest https://github.com/opencv/opencv/releases/download/4.10.0/opencv-4.10.0-windows.exe -OutFile include/opencv-4.10.0-windows.exe"
.\include\opencv-4.10.0-windows.exe -y
.\build\premake-5.0.0-beta2-windows\premake5.exe --file=.\build\premake5.lua vs2022
