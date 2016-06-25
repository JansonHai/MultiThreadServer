local fldataread = require("fldataread");
local proto = require("fldataread");

function Parse2s10000(message)
	fldataread.reset_data(message);
	proto = fldataread.read_int32(message);
	local data = {};
	data.id = fldataread.read_int32(message);
	data.name = fldataread.read_string(message);
	return data;
end

proto.set_proto2s_parse(10000, Parse2s10000);