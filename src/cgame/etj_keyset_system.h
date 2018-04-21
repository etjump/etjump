#pragma once
#include <string>
#include <vector>
#include "etj_keyset_drawer.h"
#include "cg_local.h"
#include <map>
#include "etj_cvar_master_drawer.h"
#include "etj_irenderable.h"

namespace ETJump
{
	class KeySetSystem : public IRenderable
	{
		// key set layout
		std::vector<KeySetDrawer::KeyNames> const keyLayout{
			KeySetDrawer::KeyNames::Sprint,
			KeySetDrawer::KeyNames::Forward,
			KeySetDrawer::KeyNames::Jump,
			KeySetDrawer::KeyNames::Left,
			KeySetDrawer::KeyNames::Empty,
			KeySetDrawer::KeyNames::Right,
			KeySetDrawer::KeyNames::Prone,
			KeySetDrawer::KeyNames::Backward,
			KeySetDrawer::KeyNames::Crouch,
		};
		CvarBasedMasterDrawer keySetMasterDrawer;

		std::vector<KeySetDrawer::KeyShader> createKeyPressSet(const std::string &keySetName);
		std::map<int, qhandle_t> createKeyBindSet(const std::string &keySetName);
		qhandle_t registerKeySetShader(const std::string &keySetName, const std::string &keyName);
		std::string createKeyPressSetShaderPath(const std::string &keySetName, const std::string &keyName);
		qhandle_t registerShaderNoMip(const std::string &shaderName);

	public:
		KeySetSystem(const vmCvar_t &cvar);
		~KeySetSystem();
		void addSet(const std::string &keySetName);
		void addKeyBindSet(const std::string &keySetName);
		void beforeRender() override;
		void render() const override;
	};
}
