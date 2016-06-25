local proto = require("proto");

function On10000(ctx, proto, message)
	local parse = proto.get_proto2s_parse(10000);
	local data = parse(message);
	--do other things..
	--xxoo
	--xxoo
end

proto.set_proto2s_parse(10000, On10000);