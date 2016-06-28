local filetools = require("lib/fileTools");
local tb = {};

function tb.bootstrap(path, isboot)
	print(path);
	local subdirs = filetools.get_dirs(path);
	if (0 ~= #subdirs) then
		for _,dirname in ipairs(subdirs) do
			tb.bootstrap(dirname, true);
		end
	end
	local luaFile = path .. "/bootstrap.lua";
	if (true == isboot and filetools.is_file_exist(luaFile)) then
		dofile(luaFile);
	end
end

function tb.init()
	tb.bootstrap(LUA_SCRIPT_ROOT .. "/module", false);
end

return tb;