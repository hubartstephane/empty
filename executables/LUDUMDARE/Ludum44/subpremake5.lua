-- =============================================================================
-- ROOT_PATH/executables/LUDUM/Ludum44
-- =============================================================================

local project = build:ConsoleApp()
project:DependOnLib("CHAOS")
project:AddFileToCopy("resources")
project:DependOnLib("CommonFonts")