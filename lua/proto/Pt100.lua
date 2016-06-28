local readArray = require("lib/readArray");
local proto = require("lib/proto");

function Parse2s10000(message)
	readArray.reset_data(message);
	proto = readArray.read_int32(message);
	local data = {};
	data.id = readArray.read_int32(message);
	data.name = readArray.read_string(message);
	return data;
end

proto.set_proto2s_parse(10000, Parse2s10000);