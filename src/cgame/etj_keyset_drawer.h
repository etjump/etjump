/*
 * MIT License
 * 
 * Copyright (c) 2020 ETJump team <haapanen.jussi@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
