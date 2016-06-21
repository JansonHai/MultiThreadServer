local flLib = require("flLib");
g_client_ctx = nil;
g_client_proto = nil;
g_client_message = nil;
g_handle_table = {};

function fl_handle_main(ctx, proto, message)
	g_client_ctx = ctx;
	g_client_proto = proto;
	g_client_message = message;
	if (nil != g_handle_table[proto])
	{
		handles = g_handle_table[proto];
		for _, handle in ipairs(handles) do
			flLib.resetMessagePos(message);
			handle(ctx, proto, message);
		end
	}
	g_client_ctx = nil;
	g_client_proto = nil;
	g_client_message = nil;
end





