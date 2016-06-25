local fldataread = require("fldataread");
local proto = require("proto");

function fl_handle_main(ctx, proto, message)
	handles = proto.get_proto2s_handle(proto);
	if (nil != handles) then
		for _, handle in ipairs(handles) do
			handle(ctx, proto, message);
		end
	else
		fldataread.release_data(message);
	 	fldataread.destory_data(message);
	end
end





