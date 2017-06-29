#ifndef tank_gun_h
#define tank_gun_h

#include "model.h"

namespace Models {
	namespace TankGunInternal {

	extern unsigned int gunVertices;
    extern float gunPositions[8040];
    extern float gunTexels[4020];
    extern float gunNormals[8040];
	}

	class TankGun: public Model {
		public:
			TankGun();
			virtual ~TankGun();
			virtual void drawSolid();
	};

	extern TankGun tankGun;
}

#endif
