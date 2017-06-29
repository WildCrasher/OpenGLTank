// This is a .h file for the model: hull

// Positions: 19504
// Texels: 20008
// Normals: 4309
// Faces: 10274
// Vertices: 30822
#ifndef TANK_HULL_H
#define TANK_HULL_H

#include "model.h"


namespace Models {
	namespace TankHullInternal {

	extern unsigned int hullVertices;
    extern float hullPositions[88716];
    extern float hullTexels[44358];
    extern float hullNormals[88716];
	}

	class TankHull: public Model {
		public:
			TankHull();
			virtual ~TankHull();
			virtual void drawSolid();
	};

	extern TankHull tankhull;
}


#endif
