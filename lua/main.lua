dofile("config.lua")

dofile("bootstrap.lua");

local readArray = require("readArray");
local proto = require("proto");

function fl_handle_main(ctx, protoNum, message)
	handles = proto.get_proto2s_handle(protoNum);
	local parse = proto.get_proto2s_parse(protoNum);
	local data = parse(message);
	readArray.release_data(message);
 	readArray.destory_data(message);
	if (nil != handles) then
		for _, handle in ipairs(handles) do
			handle(ctx, protoNum, data);
		end
	end
end





