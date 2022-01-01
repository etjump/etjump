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
#include "etj_keyset_drawer.h"
#include <map>

namespace ETJump
{
	class KeySetKeyBindDrawer : public KeySetDrawer
	{
	public:
		const std::map<int, qhandle_t> controlShaders;
		KeySetKeyBindDrawer(
			const std::vector<KeyShader> &keyShaders, const std::map<int, qhandle_t> controlShaders
		);
		virtual ~KeySetKeyBindDrawer() {};
	protected:
		void drawReleaseShader(qhandle_t shader, int position) const override;
		void drawPressShader(qhandle_t shader, int position) const override;
		static std::string keyNameToCommand(KeyNames keyName);
		static int getKeyCodeForName(std::string &name);
		static std::string getKeyCodeBinding(int keyCode);
		static std::string getKeyCodeFullBinding(int keyCode);
		static std::string getKeyCodeShortBinding(int keyCode);
		int checkKeyCodeRemap(int keyCode) const;
		qhandle_t checkIfKeyCodeHasShader(int keyCode) const;
	};
}
