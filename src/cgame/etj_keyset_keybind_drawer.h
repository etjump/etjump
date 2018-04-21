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
