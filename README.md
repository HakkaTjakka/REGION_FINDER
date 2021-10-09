# REGION_FINDER

C/C++ FAST REGIONS READER/CHECKER

Can be used in combination with https://github.com/HakkaTjakka/MinecraftWorldEditor (pacman.exe geo2index holland).
And https://github.com/HakkaTjakka/project-obj-dos-cmd-python-slave/blob/main/README.md
To find missing region files, create a list of marked spots in out.txt, create lat/lon list by inserting it in the project-obj TEST_LIST_TO_GEO.BAT script, then run it (result.txt) with the pacman.exe geo2index holland command, resulting in a object_array.res file, for loading the octants from object_array.txt. Msg me for help.


Usage:

region_finder.exe path_to_region_dir

or

region_finder.exe path_to_region_dir path_to_voxels_dir

(Used for comparing .mca / .vox files) BTE121 Holland).

Result is in out.txt

A '.' marks there is NO region file.

A '*' marks a region file.

When replacing a dot or asterix with another character, and then run:
 
region_finder.exe out.txt

You get a list with the r.x.z files marked. (And showing if there is a .mca or .vox file)
 
Change for other usage.

.exe compiled under Ubuntu for Windows with codeblocks and cross compiler.

Use SynWrite.6.41.2780 for column based search and replace (missing in notepad++)
