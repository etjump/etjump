/*
 * MIT License
 * 
 * Copyright (c) 2022 ETJump team <zero@etjump.com>
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
#include "etj_irenderable.h"
#include "cg_local.h"
#include <string>
#include <list>

namespace ETJump
{
	class DisplaySpeed : public IRenderable
	{
		struct StoredSpeed
		{
			int time;
			float speed;
		};

		std::list<StoredSpeed> _storedSpeeds;
		float _maxSpeed{ 0 };
		vec4_t _color;
		bool _shouldDrawShadow{ false };
		static void parseColor(const std::string &color, vec4_t &out);
		void resetMaxSpeed();
		void checkShadow();
		void startListeners();
		std::string getStatus() const;
		bool canSkipDraw() const;
		void popOldStoredSpeeds();
		float calcAvgAccel() const;
	public:
		DisplaySpeed();
		~DisplaySpeed();
		void render() const override;
		void beforeRender() override;
	};
}
