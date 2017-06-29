// This is a .h file for the model: czolg

// Positions: 19504
// Texels: 20008
// Normals: 4309
// Faces: 10274
// Vertices: 30822
#ifndef TANK_H
#define TANK_H

#include "model.h"


namespace Models {
	namespace TankInternal {

	extern unsigned int czolgVertices;
    extern float czolgPositions[88716];
    extern float czolgTexels[61644];
    extern float czolgNormals[88716];
	}

	class Tank: public Model {
		public:
			Tank();
			virtual ~Tank();
			virtual void drawSolid();
	};

	extern Tank tank;
}


#endif
