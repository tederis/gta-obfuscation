-------------------------------------------------
-- RenderWare Obfuscator
-- TEDERIs <xcplay@gmail.com>
-------------------------------------------------

*******************
** BUILDING
*******************
It should be compiled strictly targeting Win32

*******************
** SETTINGS
*******************
Notice that in the source code these definitions are default:   
	PARTS_NUM = 6 (A number of parts for assets decomposing)
	VARIANT_KEY = 23 (The key for permutation value)
	TEMP_BUFFER_SIZE = 8388608 (The max size of obfuscated file)

What is it?
---------------
This is a simple deobfuscation library is being injected into the game process.

How do use it?
------------------
To work with this you should install the ASI Loader. The compiled library RWDeobfuscator.asi should be copied into the scripts folder.
It's recommended to use Silent's ASI Loader 1.3. The link for download: http://www.gtagarage.com/mods/show.php?id=21709
