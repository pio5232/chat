#pragma once
namespace C_Network
{
	class LanSession : public C_Network::Session
	{
	public:
		LanSession() {}

		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
		virtual void OnRecv(C_Utility::CSerializationBuffer& buffer, uint16 type) override;
	};
}