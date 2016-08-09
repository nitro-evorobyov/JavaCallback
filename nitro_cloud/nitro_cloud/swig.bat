@echo off
@echo "--------------------------------------------------------"
@echo "                    nitro_cloud\swig.bat           "
@echo "--------------------------------------------------------"

@pushd %~dp0..\swigwin\
@swig.exe -c++ -java   -outdir %~dp0..\swig\java  -o "%~dp0\task_java_wrap.cxx" -DNPDFCLAPPEXPORT= -package com.nitro.cloud  "%~dp0\task.i" 
@popd