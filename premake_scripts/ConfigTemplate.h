// BEWARE: this file is generated by premake. do not edit by hand

#pragma once

// what kind of project this correspond
#define $PROJECT_BUILD_TYPE$

// defines an import/export universal token
#if !defined $PROJECT_NAME$_IS_SHARED_LIBRARY
#  define $PROJECT_NAME$_API // no import/export unless for shared library
#else
#  if defined CHAOS_BUILDING_$PROJECT_NAME$ // are we building the project, or this file is included from external ?
#    define $PROJECT_NAME$_API __declspec(dllexport)
#  else
#    define $PROJECT_NAME$_API __declspec(dllimport)
#  endif
#endif
