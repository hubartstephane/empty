-- =============================================================================
-- Some windows command reminders
-- =============================================================================

-- WTF : the os.execute(...) seems to switch / and \ on arguments for windows ...
--       causing the command to fail
--
-- MKLINK /H   new_file   src_file                  (link between files)
-- MKLINK /J   new_dir    src_dir                   (link between directories)
-- =============================================================================
-- Project description
--
-- in some commands we can use some jockers such as
--   %{sln.name}
--   %{prj.location}
--
-- they are listed below:
--  sln.name
--
--  prj.name
--  prj.language
--
--  cfg.architecture
--  cfg.buildcfg
--  cfg.buildtarget
--  cfg.kind
--  cfg.linktarget
--  cfg.longname
--  cfg.objdir
--  cfg.platform
--  cfg.shortname
--  cfg.system
--
--  file.abspath
--  file.basename
--  file.name
--  file.relpath
--       These values are available on build and link targets
--  target.abspath
--  target.basename
--  target.bundlename
--  target.bundlepath
--  target.directory
--  target.extension
--  target.name
--  target.prefix
--  target.relpath
--  target.suffix
--
-- And generic strings for portable commands:
--  {CHDIR}
--  {COPY}
--  {DELETE}
--  {ECHO}
--  {MKDIR}
--  {MOVE}
--  {RMDIR}
--  {TOUCH}

-- =============================================================================
-- some settings
-- =============================================================================

BUILD_TARGET = _ACTION
ROOT_PATH = path.getdirectory(_SCRIPT)
SOLUTION_PATH = path.join(ROOT_PATH, "solutions", BUILD_TARGET)
EXTERNAL_PATH = path.join(ROOT_PATH, "external")
BUILD_PATH = path.join(ROOT_PATH, "build", BUILD_TARGET)
BUILD_TOOLS_PATH = path.join(ROOT_PATH, "build_tools")
ZIP_PATH = path.join(ROOT_PATH, "zip")
COPY_SCRIPT = path.join(BUILD_TOOLS_PATH, "mycopy.py")
ZIP_SCRIPT = path.join(BUILD_TOOLS_PATH, "myzip.py")
DOXYGEN_SCRIPT = path.join(BUILD_TOOLS_PATH, "mydoxygen.py")
CLEAN_SCRIPT = path.join(BUILD_TOOLS_PATH, "myclean.py")

DEBUG = "DEBUG"
RELEASE = "RELEASE"
x32 = "x32"
x64 = "x64"
win32 = "win32"
win64 = "win64"

PLATFORMS = {x64} -- no more 32bit support
CONFIGS = {DEBUG, RELEASE}

-- =============================================================================
-- requirement
-- =============================================================================

require 'premake_scripts/Object'
require 'premake_scripts/Utility'
require 'premake_scripts/Log'
require 'premake_scripts/BuildSystem'
require 'premake_scripts/Project'

-- =============================================================================
-- create the some object
-- =============================================================================

build = BuildSystem:new()

require 'premake_scripts/external_premake5'

-- =============================================================================
-- options
-- =============================================================================

newoption {
	trigger = "arch",
	description = "the architecture for the build"
}

-- =============================================================================
-- Fix crashing premake for codelite
-- =============================================================================

-- see https://github.com/premake/premake-core/issues/1299

premake.override(premake.fileconfig, "getconfig", function(oldFn, fcfg, cfg)
	if fcfg.configs ~= nil then
		return fcfg.configs[cfg]
	else
		return nil
	end
end)

-- =============================================================================
-- Solution
-- =============================================================================

Log:Output("toto")

solution "Chaos"

	platforms {table.unpack(PLATFORMS)}
	configurations {table.unpack(CONFIGS)}
	
	local arch = _OPTIONS['arch']
	if arch then
		Log:Output("Architecture: " .. arch)
		architecture(arch)
	end

	location(SOLUTION_PATH) -- where the visual studio project file is been created

	if os.target() == "windows" then
		defines {"WINDOWS"}
	end

	if os.target() == "linux" then
		defines {"LINUX"}
	end

	--local sub_directories = {"libraries", "executables", "shared_resources"}
	local sub_directories = {'executables'}
	build:ProcessSubPremake(sub_directories, true) -- create sub groups
  