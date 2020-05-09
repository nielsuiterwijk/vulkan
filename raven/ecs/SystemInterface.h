#pragma once

namespace Ecs
{
	class World;

	class SystemInterface
	{
		public:

		virtual void Tick( World& World, float Delta ) = 0;
	};
}