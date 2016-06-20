local isRuning = false;
local message_queue = {};
local runState = 1;

local function main()
	while (true) do
		if (0 == runState) then
			break;
		end
		if (0 == table.getn(message_queue)) then
			isRuning = false;
			coroutine.yield();
		else
			message = table.remove(message_queue, 1);
		end

	end
end

local main_co = coroutine.create(main);

function Resume(message)
	table.insert(message_queue, message);
	if (false == isRuning) then
		isRuning = true;
		coroutine.resume(main_co);
	end
end

function Stop()
	runState = 0;
	if (false == isRuning) then
		coroutine.resume(main_co);
	end
end






