local readArray = require("lib/readArray");
local proto = require("lib/proto");

function Parse2s001(message)
	readArray.reset_data(message);
	proto = readArray.read_int32(message);
	local data = {};
	data.str = readArray.read_string(message);
	return data;
end

proto.set_proto2s_parse(1, Parse2s001);