#pragma once
namespace ETJump
{
	class IRenderable
	{
	public:
		virtual void beforeRender() = 0;
		virtual void render() const = 0;
	};
}