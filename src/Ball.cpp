#include "Core.h"

// Golden ratio and 1 / golden ratio.
#define PHI ((1.0 + Sqrt(5.0)) / 2.0)
#define IPHI (1.0 / PHI)

/*
Vertices of a regular icosahedron.
Cyclic permutations of (0, +/- 1, +/- PHI).
Note that these will be normalized to project them all onto a unit sphere.
*/
static Vec3 icosahedron[] = {
{0.0, 1.0, PHI}, {0.0, -1.0, PHI}, {0.0, 1.0, -PHI}, {0.0, -1.0, -PHI},
{PHI, 0.0, 1.0}, {PHI, 0.0, -1.0}, {-PHI, 0.0, 1.0}, {-PHI, 0.0, -1.0},
{1.0, PHI, 0.0}, {-1.0, PHI, 0.0}, {1.0, -PHI, 0.0}, {-1.0, -PHI, 0.0}};

/*
Vertices of a regular dodecahedron.
Cyclic permuations of (+/- 1, +/- 1, +/- 1) and (+/- PHI, +/- IPHI, 0).
Note that these will be normalized to project them all onto a unit sphere.
*/
static Vec3 dodecahedron[] = {
{1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {-1.0, -1.0, 1.0},
{1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0}, {1.0, -1.0, -1.0}, {-1.0, -1.0, -1.0}, 
{PHI, IPHI, 0.0}, {-PHI, IPHI, 0.0}, {PHI, -IPHI, 0.0}, {-PHI, -IPHI, 0.0}, 
{0.0, PHI, IPHI}, {0.0, -PHI, IPHI}, {0.0, PHI, -IPHI}, {0.0, -PHI, -IPHI}, 
{IPHI, 0.0, PHI}, {IPHI, 0.0, -PHI}, {-IPHI, 0.0, PHI}, {-IPHI, 0.0, -PHI}};

/*
Gets the direction vector associated with the symbol. Note that this is *not* the normal vector of the triangle,
it is actually the midpoint of the triangle. We compute it by just adding the vertex positions and normalizing.
*/
static Vec3 GetSymbolDirection(s32 symbol_id, IVec3 triangle_table[60])
{
	s32 ico_idx = triangle_table[symbol_id - 1].x;
	s32 dod_idx1 = triangle_table[symbol_id - 1].y;
	s32 dod_idx2 = triangle_table[symbol_id - 1].z;
	return Normalize(icosahedron[ico_idx] + dodecahedron[dod_idx1] + dodecahedron[dod_idx2]);
}

/*
Loads the triangle lookup table from a file. The file has 3 values for each symbol, starting with symbol ID 1.
These values let us look up which vertices from the icosahedron and dodecahedron combine to form the face for that symbol.

The first value is the index of the icosahedron vertex for the face. The other two values are indices of the
dodecahedron vertices, ordered to be clockwise around the triangle, if viewed from the outside of the ball.

This table will be different for different world seeds.
The only important property is is that it correctly places all the adjacent symbols onto adjacent faces.
I don't have a good way for you to generate this lookup table... I put together a net of our pentakis dodecahedron in
MS Paint using screenshots of all the pyramids, and built a visualizer tool in UE5 (too big to include in this repo)
to help construct the lookup table by hand.
*/
static bool ParseTriangleTable(FILE* f, IVec3 triangle_table[60])
{
	// The first line can optionally be a header, which we will skip.
	// Otherwise rewind to the start of the file.
	if (fscanf(f, "%*d,%*d,%*d\n") == 0) fscanf(f, "%*[^\n]\n");
	else fseek(f, 0, SEEK_SET);

	s32 idx = 0;
	s32 fields_parsed;
	IVec3 table[60];
	// We will set id1 and id2 to -1 if we find them.
	while (idx < ARRAYCOUNT(table) && (fields_parsed = fscanf(f, "%d,%d,%d\n", &table[idx].x, &table[idx].y, &table[idx].z)) == 3) ++idx;

	if (idx == 60)
	{
		for (s32 i = 0; i < ARRAYCOUNT(table); ++i) triangle_table[i] = table[i];
		return true;
	}
	else if (fields_parsed != EOF)
	{
		printf("Unable to parse triangle at index %d, is the line formatted correctly?\n", idx);
		return false;
	}
	else return false;
}

/*
Looks through a CSV file containing vectors from the starmapping research, to find the two
vectors corresponding to the given symbol IDs. To generate the CSV file, I used the arbitrary
IDs defined for each symbol, and hand-copied all the vectors into the file.

You only need two starmapping vectors to compute the ball orientation,
and any extras are used to verify the quality of the results.
*/
static bool FindStarmapVectors(FILE* f, s32 id1, s32 id2, Vec3* out_a, Vec3* out_b)
{
	// The first line can optionally be a header, which we will skip.
	// Otherwise rewind to the start of the file.
	if (fscanf(f, "%*d,%*lf,%*lf,%*lf\n") == 0) fscanf(f, "%*[^\n]\n");
	else fseek(f, 0, SEEK_SET);

	s32 parsed_count = 0;
	s32 id, fields_parsed;
	Vec3 v, a, b;

	// We will set id1 and id2 to -1 if we find them.
	while ((id1 >= 0 || id2 >= 0) && (fields_parsed = fscanf(f, "%d,%lf,%lf,%lf\n", &id, &v.x, &v.y, &v.z)) == 4)
	{
		if (id == id1)
		{
			id1 = -1;
			a = v;
		}
		if (id == id2)
		{
			id2 = -1;
			b = v;
		}
		++parsed_count;
	}

	// If we found both vectors, we are done! Otherwise, check for errors, and return false.
	if (id1 < 0 && id2 < 0)
	{
		*out_a = a;
		*out_b = b;
		return true;
	}
	else if (fields_parsed != EOF)
	{
		printf("Unable to parse starmap vector at index %d, is the line formatted correctly?\n", parsed_count);
		return false;
	}
	else return false;
}

s32 SolveRotation(s32 id1, s32 id2, const char* triangles_path, const char* starmap_path, Vec3 output[60])
{
	// Load the starmap file and scan it for two vectors corresponding to the symbols we want to reference.
	Vec3 starmap1, starmap2;
	FILE* starmap_file = fopen(starmap_path, "r");
	if (!starmap_file)
	{
		printf("Unable to open file %s\n", starmap_path);
		return 1;
	}
	bool success = FindStarmapVectors(starmap_file, id1, id2, &starmap1, &starmap2);
	if (!success)
	{
		printf("Unable to find both starmap vectors for symbol IDs %d and %d in file %s\n", id1, id2, starmap_path);
		return 1;
	}

	// Load the triangle lookup table.
	IVec3 triangle_table[60] = {};
	FILE* triangles_file = fopen(triangles_path, "r");
	if (!triangles_file)
	{
		printf("Unable to open file %s\n", triangles_path);
		return 1;
	}
	success = ParseTriangleTable(triangles_file, triangle_table);
	fclose(triangles_file);
	if (!success)
	{
		printf("Unable to parse triangle lookup table in file %s\n", triangles_path);
		return 1;
	}

	// Project all the vertices onto a unit sphere.
	for (s32 i = 0; i < ARRAYCOUNT(icosahedron); ++i) icosahedron[i] = Normalize(icosahedron[i]);
	for (s32 i = 0; i < ARRAYCOUNT(dodecahedron); ++i) dodecahedron[i] = Normalize(dodecahedron[i]);

	// Find the rotation we can apply to our ball to align the symbols with the known starmapping vectors.
	Vec3 forward = GetSymbolDirection(id1, triangle_table);
	Vec3 right = Normalize(Cross(forward, GetSymbolDirection(id2, triangle_table)));
	Vec3 up = Normalize(Cross(forward, right));

	Vec3 starmap_forward = Normalize(starmap1);
	Vec3 starmap_right = Normalize(Cross(starmap_forward, Normalize(starmap2)));
	Vec3 starmap_up = Normalize(Cross(starmap_forward, starmap_right));

	Quat q1 = Quat(forward, right, up);
	Quat q2 = Quat(starmap_forward, starmap_right, starmap_up);
	Quat diff_rot = q2 * Invert(q1);

	printf("\nSymbol ID,X,Y,Z\n");
	for (s32 i = 0; i < ARRAYCOUNT(triangle_table); ++i)
	{
		Vec3 v_start = GetSymbolDirection(i + 1, triangle_table);
		Vec3 v = (diff_rot * Quat(Vec4(v_start, 0.0)) * Invert(diff_rot)).xyz;
		printf("%d,%lf,%lf,%lf\n", i + 1, v.x, v.y, v.z);
		output[i] = v;
	}

	// Scan back to the first starmap vector in the file, we will parse it again to check our results.
	fseek(starmap_file, 0, SEEK_SET);
	if (fscanf(starmap_file, "%*d,%*lf,%*lf,%*lf\n") == 0) fscanf(starmap_file, "%*[^\n]\n");
	else fseek(starmap_file, 0, SEEK_SET);

	printf("\nChecking how close we are to the rest of the starmap vectors...\n\n");
	printf("Symbol ID,Similarity (ACos Angle),Computed X,Computed Y,Computed Z,Starmap X,Starmap Y,Starmap Z\n");

	s32 id;
	Vec3 v;
	while (fscanf(starmap_file, "%d,%lf,%lf,%lf\n", &id, &v.x, &v.y, &v.z) == 4)
	{
		Vec3 computed = output[id - 1];
		Vec3 starmap = v;
		double angle = Dot(computed, starmap) / (Length(computed) * Length(starmap));
		printf("%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", id, angle, computed.x, computed.y, computed.z, starmap.x, starmap.y, starmap.z);
	}
	fclose(starmap_file);
	return 0;
}