dofile( "findStarcraft.lua" );
-- path: our SC2 Install path
-- relExePath: our SC2Editor.exe relative path
print (path);

procInfo = gem.startExe(path..relExePath,"SC2Editor.exe "..gem.getArgs());
gem.sleep(15);
gem.injectDll(procInfo, gem.getCwd().."\\bin\\geHack.dll");
gem.connectServer();

function main()
	running = true;
	while true do -- event loop
		msg, param = gem.readMessage();
		if msg ~= false then
			if(msg == "exit") then
				break;
			elseif(msg == "print") then
				print (param);
			elseif(msg == "save") then
				print ("save, temp dir: "..param);
				-- perform all onsave actions, editor is now blocked waiting for message
				onSave(param);
				-- gem.messageBox("got save","gemini");
				gem.writeMessage("aftersave");
			else
				print ("unknown message: "..msg..'.'..param);
			end
		else
			print ("got 0, disconnecting");
			break;
		end
	end
end

function onSave (tmpdir)
	-- gem.messageBox("got save, tmpdir "..tmpdir,"gemini");
	-- gem.messageBox("onSave","gemini");
	print ("scriptalter: ".. gem.shellOpen("bin\\scriptalter.exe", tmpdir.."\\MapScript.galaxy"));
	-- gem.messageBox("after alter","gemini");
end

main();