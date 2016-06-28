print("config.lua start");
dofile("./lua/config.lua");
print("config.lua end");
print("path=" .. package.path);
print("cpath=" .. package.cpath);

print("bootstrap.lua start")
dofile("module/bootstrap");
print("bootstrap.lua end")

local readArray = require("lib/readArray");
local proto = require("lib/proto");

function fl_handle_main(ctx, protoNum, message)
	handles = proto.get_proto2s_handle(protoNum);
	local parse = proto.get_proto2s_parse(protoNum);
	local data = parse(message);
	readArray.release_data(message);
 	readArray.destory_data(message);
	if (nil ~= handles) then
		for _, handle in ipairs(handles) do
			handle(ctx, protoNum, data);
		end
	end
end





