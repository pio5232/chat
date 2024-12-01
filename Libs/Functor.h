#pragma once

namespace C_Utility
{
	class Functor
	{
	public:
		bool operator()(const WCHAR* first, const WCHAR* second) const;

	};
}
