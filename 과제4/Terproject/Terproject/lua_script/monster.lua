myid = 99999;

function set_uid(x)
	myid = x;
end

function event_player_move(player)
	local player_x = API_get_x(player);
	local player_y = API_get_y(player);
	local my_x = API_get_x(myid);
	local my_y = API_get_y(myid);
	if player_x == my_x then
		if player_y == my_y then
			SendHelloMessage(myid, player, "HELLO");
		end
	end
end

function event_player_bye(player)
	SendByeMessage(myid, player, "BYE");
end