local proto = {};
local _proto2s_handle = {};
local _proto2s_data_parse = {};
local _proto2c_handle = {};
local _proto2c_data_parse = {};

function proto.add_proto2s_handle(proto, func)
	proto_table = _proto2s_handle[proto];
	if (nil == proto_table) then
		proto_table = {};
	end
	table.insert(proto_table,func);
	_proto2s_handle[proto] = proto_table;
end

function proto.get_proto2s_handle(proto)
	return _proto2s_handle[proto];
end

function proto.set_proto2s_parse(proto, func)
	_proto2s_data_parse[proto] = func;
end

function proto.get_proto2s_parse(proto)
	return _proto2s_data_parse[proto];
end

function proto.add_proto2c_handle(proto, func)
	proto_table = _proto2c_handle[proto];
	if (nil == proto_table) then
		proto_table = {};
	end
	table.insert(proto_table,func);
	_proto2c_handle[proto] = proto_table;
end

function proto.get_proto2c(proto)
	return _proto2c_handle[proto];
end

function proto.set_proto2c_parse(proto, func)
	_proto2c_data_parse[proto] = func;
end

function proto.get_proto2c_parse(proto)
	return _proto2c_data_parse[proto];
end

return proto;

