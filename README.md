MeshTool is a Mesh format conversion tool for MyEngine.
It imports many formats using AssImp and exports to the .mymesh format.

Dependencies (and some commands I used for building):

Boost 1.70:
- Set BOOST_PATH as an environment variable.
- Build using VS2019 x86 Command Prompt:
	bootstrap
	b2 --with-filesystem --with-system
