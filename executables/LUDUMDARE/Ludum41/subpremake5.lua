-- =============================================================================
-- ROOT_PATH/executables/LUDUM/Ludum41
-- =============================================================================

local project = build:WindowedApp()
project:DependOnLib("CHAOS")
project:AddFileToCopy("resources")
project:DependOnLib("CommonFonts")