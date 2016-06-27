local fldataread = require("fldataread");
local proto = require("proto");

function fl_handle_main(ctx, protoNum, message)
	handles = proto.get_proto2s_handle(protoNum);
	local parse = proto.get_proto2s_parse(protoNum);
	local data = parse(message);
	fldataread.release_data(message);
 	fldataread.destory_data(message);
	if (nil != handles) then
		for _, handle in ipairs(handles) do
			handle(ctx, protoNum, data);
		end
	end
end





