-- =============================================================================
-- ROOT_PATH/executables
-- =============================================================================

for _, v in ipairs({"LUDUMDARE", "GLFW", "MISC", "TOOLS", "LUA", "SOUND", "MODEL3D", "VULKAN", "C++", "TEST_EXTERNAL_LIBRARIES" })   do
	build:ProcessSubPremake(v, true) -- create a sub groups
end
