local function run()
	isRuning = false;
end

local isRuning = false;
local co = coroutine.create(run);


function Resume()
	if (false == isRuning)
	{
		isRuning = true;
		coroutine.resume(co)
	}
end