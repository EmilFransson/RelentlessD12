if not exist Build\ mkdir Build\
cd Build\
cmake .. -G "Visual Studio 17"
#START devenv Relentless.sln
pause
EXIT /B