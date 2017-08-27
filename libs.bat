mkdir lib
mkdir maps

xcopy s2client-api\build\bin\*.lib lib\ /s /e /y

xcopy s2client-api\maps\* maps\ /s /e /y
REM xcopy include\sc2api\* project\include\sc2api\ /s /e
REM xcopy include\sc2lib\* project\include\sc2lib\ /s /e
REM xcopy include\sc2renderer\* project\include\sc2renderer\ /s /e
REM xcopy include\sc2utils\* project\include\sc2utils\ /s /e
REM xcopy build\generated\* project\include\ /s /e
REM xcopy contrib\protobuf\src\google\* project\include\google\ /s /e
exit