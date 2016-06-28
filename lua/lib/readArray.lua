local fl_read_byte_lib = require("fldataread");
local readArray = {};

function readArray.read_int8(byteArray)
	return fl_read_byte_lib.read_int8(byteArray);
end

function readArray.read_int16(byteArray)
	return fl_read_byte_lib.read_int16(byteArray);
end

function readArray.read_int32(byteArray)
	return fl_read_byte_lib.read_int32(byteArray);
end

function readArray.read_int64(byteArray)
	return fl_read_byte_lib.read_int64(byteArray);
end

function readArray.read_float(byteArray)
	return fl_read_byte_lib.read_float(byteArray);
end

function readArray.read_double(byteArray)
	return fl_read_byte_lib.read_double(byteArray);
end

function readArray.read_string(byteArray)
	return fl_read_byte_lib.read_string(byteArray);
end

function readArray.reset_data(byteArray)
	return fl_read_byte_lib.reset_data(byteArray);
end

function readArray.release_data(byteArray)
	return fl_read_byte_lib.release_data(byteArray);
end

function readArray.destory_data(byteArray)
	return fl_read_byte_lib.destory_data(byteArray);
end

return readArray;