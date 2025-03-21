#pragma once

namespace C_Network
{
	class LobbySession : public C_Network::Session
	{
	public:
		LobbySession() : _userId(0), _isReady(false)
		{
			_curRoom = std::weak_ptr<Room>();
		}

		void SetRoom(std::weak_ptr<Room> room) { _curRoom = room; }
		std::weak_ptr<Room> GetRoom() const { return _curRoom; }

		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type) override;
	public:
		ULONGLONG _userId;
		bool _isReady;
	private:
		std::weak_ptr<Room> _curRoom;
	};
}	