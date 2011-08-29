dofile( "findStarcraft.lua" );
-- path: our SC2 Install path
-- relExePath: our SC2Editor.exe relative path
print (path);

procInfo = gem.startExe(path..relExePath,"SC2Editor.exe "..gem.getArgs());
gem.sleep(5);
gem.injectDll(procInfo, gem.getCwd().."\\bin\\geHack.dll");