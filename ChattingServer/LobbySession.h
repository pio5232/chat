#pragma once

namespace C_Network
{
	class LobbySession : public C_Network::Session
	{
	public:
		LobbySession() : _userId(0), _isReady(false)
		{
			_curRoom = std::weak_ptr<Room>();
			_aliveLobbySessionCount.fetch_add(1);
		}

		~LobbySession()
		{
			_aliveLobbySessionCount.fetch_sub(1);
		}

		// For Log
		static int GetAliveLobbySessionCount() { return _aliveLobbySessionCount.load(); }

		void SetRoom(std::weak_ptr<Room> room) { _curRoom = room; }
		std::weak_ptr<Room> GetRoom() const { return _curRoom; }

		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type) override;
	public:
		ULONGLONG _userId;
		bool _isReady;

	private:
		static std::atomic<int> _aliveLobbySessionCount;
		std::weak_ptr<Room> _curRoom;
	};
}	