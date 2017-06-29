#ifndef tank_turret_h
#define tank_turret_h

#include "model.h"

namespace Models {
	namespace TankTurretInternal {

	extern unsigned int turretVertices;
    extern float turretPositions[26532];
    extern float turretTexels[13266];
    extern float turretNormals[26532];
	}

	class TankTurret: public Model {
		public:
			TankTurret();
			virtual ~TankTurret();
			virtual void drawSolid();
	};

	extern TankTurret tankTurret;
}

#endif