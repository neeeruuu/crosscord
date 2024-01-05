@echo off

cd /D %~dp0

mkdir proj
cd proj

cmake ../

cd ..