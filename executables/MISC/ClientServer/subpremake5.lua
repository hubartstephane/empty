-- =============================================================================
-- ROOT_PATH/executables/MISC/ClientServer
-- =============================================================================

local project = build:ConsoleApp()
project:DependOnLib("CHAOS")
project:AddFileToCopy("testfile.txt")