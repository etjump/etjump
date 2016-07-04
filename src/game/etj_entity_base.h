#pragma once

typedef struct gentity_s gentity_t;

namespace ETJump
{
	class Entity
	{
	public:
		explicit Entity(gentity_t *ent = nullptr);
		virtual ~Entity();

		gentity_t *entity() const;
	private:
		gentity_t *_entity;
	};
}