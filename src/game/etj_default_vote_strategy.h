#pragma once
#include "etj_ivote_strategy.h"

namespace ETJump
{
	class DefaultVoteStrategy : public IVoteStrategy
	{
	public:
		Result getVoteResult(Input input) override;
	};
}
