#!/bin/bash
echo "export MGBA_EXECUTABLE=/mnt/e/Emulation/GBA/Modding/Pokemon/Decomp/tools/mGba-0.10.2-win64/mGBA.exe" >> ~/.bashrc
echo "export TODAYS_IP=$(ipconfig.exe | grep 'vEthernet (WSL)' -A4 | cut -d":" -f 2 | tail -n1 | sed -e 's/\s*//g')" >> ~/.bashrc
source ~/.bashrc
echo $TODAYS_IP