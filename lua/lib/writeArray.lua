local fl_write_byte_lib = require("fldatawrite");
local writeByteArray = {};

function writeByteArray.write_int8(byteArray, num)
	fl_write_byte_lib.write_int8(byteArray, num);
end

function writeByteArray.write_int16(byteArray, num)
	fl_write_byte_lib.write_int16(byteArray, num);
end

function writeByteArray.write_int32(byteArray, num)
	fl_write_byte_lib.write_int32(byteArray, num);
end

function writeByteArray.write_int64(byteArray, num)
	fl_write_byte_lib.write_int64(byteArray, num);
end

function writeByteArray.write_float(byteArray, num)
	fl_write_byte_lib.write_float(byteArray, num);
end

function writeByteArray.write_double(byteArray, num)
	fl_write_byte_lib.write_double(byteArray, num);
end

function writeByteArray.write_string(byteArray, str)
	fl_write_byte_lib.write_string(byteArray, str);
end

function writeByteArray.new_data()
	fl_write_byte_lib.new_data();
end

function writeByteArray.reset_data(byteArray)
	fl_write_byte_lib.reset_data(byteArray);
end

function writeByteArray.release_data(byteArray)
	fl_write_byte_lib.release_data(byteArray);
end

function writeByteArray.destory_data(byteArray)
	fl_write_byte_lib.destory_data(byteArray);
end

return writeByteArray;