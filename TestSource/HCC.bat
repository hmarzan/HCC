@echo off
echo H++ Compiler Driver batch support
echo.
echo.
REM ********************************************BEGIN customize
SET PATH=%PATH%;C:\Masm615;C:\HCC\HCC\HCCAsmLib
SET INCLUDE=C:\Masm615\INCLUDE
SET LIB=C:\Masm615\LIB
CD "C:\hcc\hcc\TestSource"
@echo on
hcc unittesting1.hpp /cl /w /GH /GZ /A /Oy
@echo off
REM ********************************************END customize
echo.
