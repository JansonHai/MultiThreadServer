local proto = require("lib/proto");
--local writeArray = require("lib/writeArray");

function On001(ctx, proto, message)
--	array = writeArray.new_data();
--	writeArray.write_string(array, message.str);
--	writeArray.destory_data(array)
	print(message.str);
end

proto.add_proto2s_handle(1, On001);