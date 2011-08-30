dofile( "findStarcraft.lua" );
-- path: our SC2 Install path
-- relExePath: our SC2Editor.exe relative path
print (path);

procInfo = gem.startExe(path..relExePath,"SC2Editor.exe "..gem.getArgs());
gem.sleep(15);
gem.injectDll(procInfo, gem.getCwd().."\\bin\\geHack.dll");
gem.connectServer();

running = true;
while running do
	while true do -- event loop
		msg = gem.readMessage()
		if msg ~= false then
			if(msg == "exit") then
				running = false;
				break;
			else
				print ("message: "..msg);
			end
		else
			break;
		end
	end
end