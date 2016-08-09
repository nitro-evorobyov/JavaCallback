%module(directors="1") nitro_cloud

%{
#include "task_result.h"
#include "task.h"
#include "task_swig.h"
%}


%include std_string.i
%include std_wstring.i
%include typemaps.i

%feature("director"); 

%insert("runtime") %{
#define SWIG_JAVA_ATTACH_CURRENT_THREAD_AS_DAEMON
%} 

%include "task_result.h"
%include "task.h"
%include "task_swig.h"

