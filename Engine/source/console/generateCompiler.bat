@echo off
call bison.bat %1 CMD cmdgram.cpp CMDgram.y . cmdgram.h
%2 -PCMD -oCMDscan.cpp CMDscan.l
