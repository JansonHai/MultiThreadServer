local fl_filetoollib = require("flfiletools");
local filetools = {};

function filetools.pwd()
	return fl_filetoollib.pwd();
end

function filetools.cd(path)
	return fl_filetoollib.cd(path);
end

function filetools.get_file_ext(file)
	return fl_filetoollib.getExt(file);
end

function filetools.get_dirs(path)
	return fl_filetoollib.getDirs();
end

function filetools.get_files(path)
	return fl_filetoollib.getFiles(path);
end

function filetools.get_all(path)
	return fl_filetoollib.getAll(path);
end

function filetools.is_file(file)
	return fl_filetoollib.isFile(file);
end

function filetools.is_file_exist(file)
	return fl_filetoollib.isFileExist(file);
end

function filetools.is_dir(path)
	return fl_filetoollib.isDir(path);
end

return filetools;