// BEWARE: this file is generated by premake. do not edit by hand#pragma once// what kind of project this correspond#define CHAOS2_IS_SHARED_LIBRARY// defines an import/export universal token#if !defined CHAOS2_IS_SHARED_LIBRARY#  define CHAOS2_API // no import/export unless for shared library#else#  if defined CHAOS_BUILDING_CHAOS2 // are we building the project, or this file is included from external ?#    define CHAOS2_API __declspec(dllexport)#  else#    define CHAOS2_API __declspec(dllimport)#  endif#endif