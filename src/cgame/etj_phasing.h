#pragma once
#include <tuple>
#include <vector>

namespace ETJump
{
	typedef struct RemapPair {
		std::string from;
		std::string to;
	};

	class PhaseRemapper
	{
	public:
		static const int maxRemaps = 32;
		static void initialize();

		PhaseRemapper();
		~PhaseRemapper();

		void update(int eFlags);

	private:
		static std::vector<RemapPair> _remapsA;
		static std::vector<RemapPair> _remapsB;
		static bool _anyRemaps;

		bool _isActiveA;
		bool _isActiveB;
	};
}