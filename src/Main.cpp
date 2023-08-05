
#include "Core.h"

#define GMATH_IMPLEMENTATION
#include "GMath.h"


/*
The star-mapping research gives us a few of these vectors directly, and we visited enough pyramids to work out
which symbols are adjacent to each other on this "ball" (technically a pentakis dodecahedron, but whatever).

This program can work out how to orient that ball so that it lines up with the vectors we already know,
and that gives us a list of vectors. Every symbol represents a direction vector in 3D space.
*/

/*
We've arbitrarily assigned each symbol an ID, just by looking at the texture for the symbols in the mod data,
and numbering them starting from 1. The first vector in this array corresponds to ID 1, and it counts up from there.
Only the first 60 symbols have a vector, but there are 4 more symbols at the stargate, and 4 more spaces in the 2D map.
*/

/*
The direction vector we are trying to produce. The exploration journal has a big lore entry of a computer log, and it mentions that we were
traveling in the direction opposite this one when we crashed. Our assumption is that we need to travel in this (reversed) direction to get home!
*/
static Vec3 desired_vector = {0.43134830415853,-0.77703970963888,0.4584189461005};

// I'm using symbol IDs 14 and 13 from starmapping in order to compute the vectors, but there may be a more precise combination.

s32 main(s32 argc, const char* argv[])
{
	// Call the program either as "exe_name interburbulate" or "exe_name interburbulate file_path" to solve interburbul puzzles.
	// If you don't specify a file path, it will try to read from "burbs.csv".
	if (argc > 1 && strcmp(argv[1], "interburbulate") == 0)
	{
		const char* file_path = (argc == 3) ? argv[2] : "burbs.csv";
		return RunInterburbul(file_path);
	}
	
	// Call the program as "exe_name ball id1 id2" or "exe_name ball id1 id2 triangles_path starmap_path" to solve the symbol direction vectors.
	// If you don't specify file paths, it will try to read from "triangles.csv" and "starmap.csv".
	if (argc > 3 && strcmp(argv[1], "ball") == 0)
	{
		s32 symbol1 = atoi(argv[2]);
		s32 symbol2 = atoi(argv[3]);
		const char* triangles_path = (argc > 4) ? argv[4] : "triangles.csv";
		const char* starmap_path = (argc > 5) ? argv[5] : "starmap.csv";
		printf("Computing symbol vectors using starmap symbol IDs %d and %d, from triangles file %s, starmap file %s\n", symbol1, symbol2, triangles_path, starmap_path);

		Vec3 symbol_vectors[60] = {};
		return SolveRotation(symbol1, symbol2, triangles_path, starmap_path, symbol_vectors);
	}

	printf("Valid Usage:\ninterburbulate file_path\nball symbol1 symbol2 triangles_path starmap_path\n");
	return 1;
}
