-- =============================================================================
-- ROOT_PATH/executables/linux
-- =============================================================================

for _, v in ipairs({"test1", "test_glm", "test_tinyxml2", "test_json", "test_glfw", "test_freeimage", "test_classes", "test_lua" })   do
	build:ProcessSubPremake(v, true) -- create a sub groups
end
