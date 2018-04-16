#pragma once
#include "cg_local.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "etj_irenderable.h"
#include <vector>

namespace ETJump
{
	class KeySetDrawer : public IRenderable
	{
	public:
		enum class KeyNames
		{
			Empty,
			Forward,
			Backward,
			Right,
			Left,
			Jump,
			Crouch,
			Sprint,
			Prone,
			LeanRight,
			LeanLeft,
			Walk,
			Talk,
			Activate,
			Attack,
			Attack2,
			Reload,
			Zoom
		};

		struct KeyShader
		{
			KeyNames key;
			qhandle_t press;
			qhandle_t release;
		};

		KeySetDrawer(const std::vector<KeyShader> &keyShaders);
		virtual ~KeySetDrawer() {};
		void render() const override;
		void beforeRender() override {};
		static std::string keyNameToString(KeyNames keyName);
	protected:
		struct Point2d
		{
			float x;
			float y;
		};

		struct KeyAttrs
		{
			vec4_t color;
			vec4_t shadowColor;
			float size;
			Point2d origin;
			bool shouldDrawShadow;
		};

		KeyAttrs attrs;
		const std::vector<KeyShader> keyShaders;

		void initListeners();
		void initAttrs();
		void updateKeysColor(const char *str);
		void updateKeysSize(float size);
		void updateKeysOrigin(float x, float y);
		void updateKeysShadow(bool shouldDrawShadow);
		void updateKeysShadowColor(const vec4_t shadowColor);
		virtual void drawShader(qhandle_t shader, int position) const;
		virtual void drawPressShader(qhandle_t shader, int position) const;
		virtual void drawReleaseShader(qhandle_t shader, int position) const;

		static int isKeyPressed(KeyNames key);
		static playerState_t* getValidPlayerState();

		template <int Cols>
		static Point2d calcGridPosition(float cellSize, int cellIndex)
		{
			return{
				cellSize * (cellIndex % Cols),
				cellSize * static_cast<int>(cellIndex / Cols)
			};
		}
	};
}
