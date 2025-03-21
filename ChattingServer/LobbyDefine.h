#include <iostream>

namespace C_Network
{
	class LobbySession;
	class LanSession;
}

using LobbySessionPtr = std::shared_ptr<C_Network::LobbySession>;
using LanSessionPtr = std::shared_ptr<C_Network::LanSession>;