local filetools = require("fileTools");
local bootstrap = {};

local function bootstrap(path, boot)
	local subdirs = filetools.get_dirs(path);
	if (0 ~= table.getn(subdirs)) then
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
	bootstrap("module", false);
end

return bootstrap;