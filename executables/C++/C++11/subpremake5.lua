-- =============================================================================
-- ROOT_PATH/executables/C++/C++11
-- =============================================================================

local project = build:ConsoleApp()
project:DependOnLib("CHAOS")
project:DependOnLib("TESTDLL")