# REGION_FINDER

C/C++ FAST REGIONS READER/CHECKER

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
