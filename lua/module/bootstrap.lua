local filetools = require("lib/fileTools");
local bootstrap = {};

local function bootstrap(path, boot)
	print(path);
	local subdirs = filetools.get_dirs(path);
	if (0 ~= #subdirs) then
		for _,dirname in ipairs(subdirs) do
			bootstrap(dirname, true);
		end
	end
	local luaFile = path .. "/bootstrap.lua";
	if (true == boot and filetools.is_file_exist(luaFile)) then
		dofile(luaFile);
	end
end

function bootstrap.init()
	bootstrap(LUA_SCRIPT_ROOT .. "/module", false);
end

return bootstrap;