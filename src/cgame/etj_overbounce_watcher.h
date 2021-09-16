/*
 * MIT License
 * 
 * Copyright (c) 2021 ETJump team <zero@etjump.com>
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
#include <array>
#include <map>
#include <string>
#include <vector>
#include "etj_irenderable.h"
#include "../game/q_shared.h"

typedef struct playerState_s playerState_t;

namespace ETJump
{
	class ClientCommandsHandler;

	class OverbounceWatcher : public IRenderable
	{
	public:
		using Coordinate = std::array<float, 3>;

		explicit OverbounceWatcher(ClientCommandsHandler *clientCommandsHandler);
		~OverbounceWatcher();

		void render() const override;
		void beforeRender() override;

		vec4_t _color;

	private:
		ClientCommandsHandler *_clientCommandsHandler;

		std::map<std::string, Coordinate> _positions;

		// Currently displayed position
		Coordinate *_current;

		// saves the position with name
		void save(const std::string& name, Coordinate coordinate);

		// stop displaying anything
		void reset();

		// loads the position to currently displayed position
		// if position is not found, returns false
		bool load(const std::string& name);

		// lists all available positions
		std::vector<std::string> list() const;

		static const playerState_t *getPlayerState();

		// checks if it's possible to overbounce with current velocity and height 
		static bool isOverbounce(float vel, float currentHeight,
			float finalHeight, float rintv,
			float psec, int gravity);

		// checks if current surface has disabled overbounce
		static bool surfaceAllowsOverbounce(trace_t* trace);
	};
}
