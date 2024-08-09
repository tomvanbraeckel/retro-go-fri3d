#./rg_tool.py --target fri3d-2024 --fatsize 5376K build-img launcher retro-core prboom-go 

#export PROJECT_VER=1.3_zips_including_coverart
#./rg_tool.py --target fri3d-2024 --fatsize 0x670000 build-img launcher retro-core prboom-go 

#export PROJECT_VER=1.3_zips_including_coverart_oldpart
#./rg_tool.py --target fri3d-2024 --fatsize 5376K build-img launcher retro-core prboom-go 

# this still reverts to the tag thing that rg_tool.py sets!
#export RG_BUILD_VERSION=v1.2-55-fri3d-1.5
#export RG_PROJECT_VERSION="$RG_BUILD_VERSION"
#export PROJECT_VER="$RG_BUILD_VERSION"
#./rg_tool.py --target fri3d-2024 --fatsize 0x670000 build-img launcher retro-core prboom-go 

unset RG_BUILD_VERSION
unset RG_PROJECT_VERSION
unset PROJECT_VER
./rg_tool.py --target fri3d-2024 --fatsize 0x670000 build-img launcher retro-core prboom-go 
