#include "etj_default_vote_strategy.h"
#include <cmath>

ETJump::IVoteStrategy::Result ETJump::DefaultVoteStrategy::getVoteResult(Input input)
{
	Result result{};
	result.status = Status::Wait;

	// wait atleast 3 seconds before passing
	if (input.duration < input.minWait)
	{
		return result;
	}

	if (input.votePercent < 0)
	{
		result.status = Status::Pass;
		return result;
	}

	if (input.votePercent > 100) input.votePercent = 100;

	auto requiredPlayers = static_cast<int>(ceil((input.numPlayers / 100.0f) * input.votePercent));
	if (input.yesVotes >= requiredPlayers)
	{
		result.status = Status::Pass;
		return result;
	}

	if (input.numPlayers - input.noVotes < requiredPlayers || input.duration >= input.maxDuration)
	{
		// fail
		result.status = Status::Fail;
		return result;
	}

	result.status = Status::Wait;
	return result;
}
