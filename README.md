# Gemini project

## What it is?
It is external program, that runs Galaxy Editor and equip it with additional features transparently to end user.\\
It is still in very early stage of development.
## How it works?
It creates new process for editor, perform a [dll injection](http://en.wikipedia.org/wiki/DLL_injection) by process memory swap and wait for signals. Anything is controlled from external [Lua](http://www.lua.org/) script, what makes it very expandable. If you want more details or want to contribute, check source.
## I want one! \(download\)
You can download latest build [here](http://dl.dropbox.com/u/14935971/gemini/gemini_build0.2.7z), but __be aware__, it is not well tested and **i can't guarantee it wont break your map**. We don't have many compatible extensions for this moment too.

## More about demo extension
(aka scriptalter.exe)
When you save your map, it adds additional trigger to MapScript. You should get "Gemini injection successful" message at third second of game time.
