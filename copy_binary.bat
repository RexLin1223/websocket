@echo off
mkdir bin & cd .\bin
mkdir x86 & cd .\x86
mkdir release & mkdir debug
cd ..
mkdir x64 & cd .\x64
mkdir release & mkdir debug
cd .. & cd ..

echo Y | xcopy /Y /C .\Release\LIBEAY32.dll .\bin\x86\release
echo Y | xcopy /Y /C .\Release\SSLEAY32.dll .\bin\x86\release
echo Y | xcopy /Y /C .\Release\WebsocketKeyServer.dll .\bin\x86\release
echo Y | xcopy /Y /C .\Release\WebsocketKeyServer.pdb .\bin\x86\release

echo Y | xcopy /Y /C .\Debug\LIBEAY32.dll .\bin\x86\debug
echo Y | xcopy /Y /C .\Debug\SSLEAY32.dll .\bin\x86\debug
echo Y | xcopy /Y /C .\Debug\WebsocketKeyServer.dll .\bin\x86\debug
echo Y | xcopy /Y /C .\Debug\WebsocketKeyServer.pdb .\bin\x86\debug

echo Y | xcopy /Y /C .\x64\Release\LIBEAY32.dll .\bin\x64\release
echo Y | xcopy /Y /C .\x64\Release\SSLEAY32.dll .\bin\x64\release
echo Y | xcopy /Y /C .\x64\Release\WebsocketKeyServer.dll .\bin\x64\release
echo Y | xcopy /Y /C .\x64\Release\WebsocketKeyServer.pdb .\bin\x64\release

echo Y | xcopy /Y /C .\x64\Debug\LIBEAY32.dll .\bin\x64\debug
echo Y | xcopy /Y /C .\x64\Debug\SSLEAY32.dll .\bin\x64\debug
echo Y | xcopy /Y /C .\x64\Debug\WebsocketKeyServer.dll .\bin\x64\debug
echo Y | xcopy /Y /C .\x64\Debug\WebsocketKeyServer.pdb .\bin\x64\debug

