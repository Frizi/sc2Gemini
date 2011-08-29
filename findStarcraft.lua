print "Gemini 0.1"

key = "HKEY_CURRENT_USER\\Software\\Blizzard Entertainment\\StarCraft II Editor\\Gemini"
relExePath = "\\Support\\SC2Editor.exe";

path = "";

if gem.regKeyExists(key,"Sc2InstallPath") then
	path = gem.getRegPair(key,"Sc2InstallPath");
end

-- try to guess some patches
if not gem.exists(path..relExePath) then
	path = "D:\\Games\\StarCraft II";
end
if not gem.exists(path..relExePath) then
	path = "C:\\Games\\StarCraft II";
end
if not gem.exists(path..relExePath) then
	path = "C:\\Program Files (x86)\\StarCraft II";
end
if not gem.exists(path..relExePath) then
	path = "C:\\Program Files\\StarCraft II";
end
if not gem.exists(path..relExePath) then
	path = "D:\\Program Files (x86)\\StarCraft II";
end
if not gem.exists(path..relExePath) then
	path = "D:\\Program Files\\StarCraft II";
end
if not gem.exists(path..relExePath) then
	path = "D:\\Games\\StarCraft II eng"; -- for me and maybe others
end
if not gem.exists(path..relExePath) then
	path = gem.browse("Select Directory Starcraft II is Installed to");
	if not gem.exists(path..relExePath) then
		gem.messageBox("Could not locate SC2Editor.exe","Gemini",false);
	end
end

if gem.exists(path..relExePath) then
	gem.setRegString(key,"Sc2InstallPath",path);
end
