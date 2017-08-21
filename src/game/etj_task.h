#pragma once
#include <functional>
#include <future>

template<typename R>
bool is_ready(std::future<R> const& f)
{
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

namespace ETJump
{
	class AbstractTask
	{
	public:
		virtual ~AbstractTask() = default;
		virtual bool isReady() = 0;
		virtual void execute() = 0;
	};

	template<typename T>
	class Task : public AbstractTask
	{
	public:
		Task<T>(std::future<T> future, std::function<void(T)> onCompletion) 
			: _future(std::move(future)), _onCompletion(onCompletion) {}
		virtual ~Task() {};

		bool isReady() override {
			return is_ready(_future);
		};

		void execute() override {
			_onCompletion(_future.get());
		}
	private:
		std::future<T> _future;
		std::function<void(T)> _onCompletion;
	};
}
