@Echo Off
SET BAK_PATH=%PATH%
SET PLATFORM=x86
SET CMAKE_PATH=D:\Program\cmake-3.29.2\bin;
SET DEVENV_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE;
SET XCOPY=xcopy /E /Y /I /Q
SET PATH=%PATH%;%CMAKE_PATH%;%DEVENV_PATH%

CMake .

:quit
SET XCOPY=
SET PATH=%BAK_PATH%
