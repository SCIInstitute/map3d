@echo off
rem map3d-fids1.sh
rem A new (for version 6.5) script to show how we can display fiducials
rem Make sure to advance the time signals to close to activation time and
rem then observe the contours over shaded potentials.

..\map3d.exe -f ..\geom\socks\490sock.fac -p ..\data\socks\10jan01\rsm10jan01-cs-0004.mat -s 1 712 -i 1 -nco 10 -w -as 171 671 250 750 -al 1 170 500 750 -t 236 -at 672 850 600 750 -t 211 -at 672 850 450 600 -t 394 -at 672 850 300 450
