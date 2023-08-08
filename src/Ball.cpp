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

// This epsilon is probably too small to be practically useful, but it seems like the math works,
// and it does not work if this value is bigger.
#define EPSILON 0.000000000000001

#define SUBDIVISION_COUNT 7
#define SUBDIVISION_AMOUNT 8
/*
I don't really remember everything about how this works. The method is taken mostly from here,
and is based on a method described in the book "Real Time Rendering":
https://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/

This is modified to assume that the ray starts at the origin, so d is a direction vector, and
we don't care about the length of the ray. Optionally returns the intersection point as
cartesian and barycentric coordinates.
*/
bool RayTriangleIntersect(Vec3 d, Vec3 v0, Vec3 v1, Vec3 v2, Vec3* out, double* out_u, double* out_v)
{
	Vec3 e1 = v1 - v0;
	Vec3 e2 = v2 - v0;
	Vec3 h = Cross(d, e2);
	double a = Dot(e1, h);
	if (a > -EPSILON && a < EPSILON) return false;
	double f = 1.0 / a;
	double u = f * Dot(-v0, h);
	if (u < 0.0 || u > 1.0) return false;
	Vec3 q = Cross(-v0, e1);
	double v = f * Dot(d, q);
	if (v < 0.0 || u + v > 1.0) return false;

	double t = f * Dot(e2,q);
	if (t > EPSILON)
	{
		if (out) *out = d * t;
		if (out_u) *out_u = u;
		if (out_v) *out_v = v;
		return true;
	}
	else return false;
}

/*
Gets the direction vector associated with the symbol. Note that this is *not* the normal vector of the face,
it is actually the centroid. We compute it by just adding the vertex positions and normalizing.
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
Parses the mapping from triangle index to symbol ID. The triangle index starts with 0 at the top,
counting up as we travel down and to the right. The bottom right corner is index 14. Then we start
back towards the top with index 15, counting down and to the right again, and repeating.
*/
bool ParseMapping2D(FILE* f, s32 output[64])
{
	// The first line can optionally be a header, which we will skip.
	// Otherwise rewind to the start of the file.
	if (fscanf(f, "%*d\n") == 0) fscanf(f, "%*[^\n]\n");
	else fseek(f, 0, SEEK_SET);

	s32 idx = 0;
	s32 fields_parsed;
	s32 table[64];
	while (idx < ARRAYCOUNT(table) && (fields_parsed = fscanf(f, "%d\n", &table[idx]) == 1)) ++idx;

	if (idx == 64)
	{
		for (s32 i = 0; i < ARRAYCOUNT(table); ++i) output[i] = table[i];
		return true;
	}
	else if (fields_parsed != EOF)
	{
		printf("Unable to parse 2d mapping value at index %d, is the line formatted correctly?\n", idx);
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

// Gets two barycentric coordinates of a point p, on a triangle formed by v0, v1, and v2.
// You can get the third barycentric coordinates via 1.0 - u - v.
// If the point is outside the triangle, one of the three coordinates will be negative.
Vec2 CartesianToBarycentric(Vec3 p, Vec3 v0, Vec3 v1, Vec3 v2)
{
	Vec3 e1 = v1 - v0;
	Vec3 h = Cross(p, v2 - v0);
	double a = Dot(e1, h);
	if (a > -EPSILON && a < EPSILON) assert(false);
	return Vec2(Dot(-v0, h), Dot(p, Cross(-v0, e1))) * (1.0 / a);
}

// Converts barycentric coordinates into cartesian coordinates.
// The expected barycentric coordinates are the same components and
// in the same order as you would receive from CartesianToBarycentric().
Vec3 BarycentricToCartesian(Vec2 bary, Vec3 v0, Vec3 v1, Vec3 v2)
{
	return v1 * bary.u + v2 * bary.v + v0 * (1.0 - bary.u - bary.v);
}

// A lookup table to convert from one somewhat arbitrary indexing of subdivided triangles into the three vertices
// that form the triangle.
IVec3 bary_lut[] = {
	{0, 1, 9},
	{10, 9, 1},
	{10, 2, 1},
	{11, 10, 2},
	{2, 3, 11},
	{12, 11, 3},
	{3, 4, 12},
	{13, 2, 4},
	{4, 5, 13},
	{14, 13, 5},
	{5, 6, 14},
	{15, 14, 6},
	{6, 7, 15},
	{16, 15, 7},
	{7, 8, 16},
	{9, 10, 17},
	{18, 17, 10},
	{10, 11, 18},
	{19, 18, 11},
	{11, 12, 19},
	{20, 19, 12},
	{12, 13, 20},
	{21, 20, 13},
	{13, 14, 21},
	{22, 21, 14},
	{14, 15, 22},
	{23, 22, 15},
	{15, 16, 23},
	{17, 18, 24},
	{25, 24, 18},
	{18, 19, 25},
	{26, 25, 19},
	{19, 20, 26},
	{27, 26, 20},
	{20, 21, 27},
	{28, 27, 21},
	{21, 22, 28},
	{29, 28, 22},
	{22, 23, 29},
	{24, 25, 30},
	{31, 30, 25},
	{25, 26, 31},
	{32, 31, 26},
	{26, 27, 32},
	{33, 32, 27},
	{27, 28, 33},
	{34, 33, 28},
	{28, 29, 34},
	{30, 31, 35},
	{36, 35, 31},
	{31, 32, 36},
	{37, 36, 32},
	{32, 33, 37},
	{38, 37, 33},
	{33, 34, 38},
	{35, 36, 39},
	{40, 39, 36},
	{36, 37, 40},
	{41, 40, 37},
	{37, 38, 41},
	{39, 40, 42},
	{43, 42, 40},
	{40, 41, 43},
	{42, 43, 44}};

/*
Subdivides a triangle formed by v0, v1, and v2 into 64 smaller ones,
placing the 45 vertices which compose it into the output.
*/
bool SubdivideTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 out_vertices[45])
{
	s32 count = 0;
	for (s32 j = 0; j <= SUBDIVISION_AMOUNT; ++j)
	{
		for (s32 i = 0; i <= SUBDIVISION_AMOUNT; ++i)
		{
			Vec2 bary = Vec2(i, j) * (1.0 / SUBDIVISION_AMOUNT);
			if (bary.u + bary.v <= 1.0)
			{
				if (++count > 45) break;
				Vec3 out = BarycentricToCartesian(bary, v0, v1, v2);
				out_vertices[count - 1] = out;
			}
		}
	}
	if (count == 45) return true;
	else
	{
		printf("Tried to subdivide face, but did not get the right number of vertices.\n");
		return false;
	}
}

/*
Finds which triangle in a subdivided one intersects the desired vector, by raycasting.
*/
bool FindIntersectedTriangle(Vec3 desired, Vec3 subdivided_vertices[45], IVec3* result, s32* out_idx)
{
	for (s32 i = 0; i < ARRAYCOUNT(bary_lut); ++i)
	{
		Vec3 v0 = subdivided_vertices[bary_lut[i].x];
		Vec3 v1 = subdivided_vertices[bary_lut[i].y];
		Vec3 v2 = subdivided_vertices[bary_lut[i].z];

		Vec3 out = {};
		Vec2 out_bary = {};
		bool does_intersect = RayTriangleIntersect(Normalize(desired), v0, v1, v2, &out, &out_bary.u, &out_bary.v);
		if (does_intersect)
		{
			printf("Intersection found with subdivided triangle idx %d:\nV0: (%.15f, %.15f, %.15f)\nV1: (%.15f, %.15f, %.15f)\nV2: (%.15f, %.15f, %.15f)\n", i, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
			printf("Intersection Point: (%.15f, %.15f, %.15f), U=%.15f, V=%.15f\n", out.x, out.y, out.z, out_bary.u, out_bary.v);
			out = Normalize(out);
			printf("Normalized: (%.15f, %.15f, %.15f)\n", out.x, out.y, out.z);
			*result = bary_lut[i];
			*out_idx = i;
			return true;
		}
	}
	return false;
}


/*
Solves the last 7 symbols in the combination by raycasting against each subdivided triangle, subdividing the hit triangle,
and repeating.
*/
bool SolveViaRaycast(Vec3 desired, Vec3 v0, Vec3 v1, Vec3 v2, IVec3 triangle_table[60], s32 out_indices[SUBDIVISION_COUNT])
{	
	IVec3 indices = {};
	s32 count = 0;
	Vec3 subdivided_vertices[45] = {};
	if (!SubdivideTriangle(v0, v1, v2, subdivided_vertices))
	{
		printf("Unable to subdivide triangle, aborting!\n");
		return false;
	}

	s32 i;
	s32 output_idx = 0;
	while (count++ < SUBDIVISION_COUNT && FindIntersectedTriangle(desired, subdivided_vertices, &indices, &i))
	{
		v0 = subdivided_vertices[indices.x];
		v1 = subdivided_vertices[indices.y];
		v2 = subdivided_vertices[indices.z];

		out_indices[output_idx++] = i;
		if (!SubdivideTriangle(v0, v1, v2, subdivided_vertices))
		{
			printf("Unable to subdivide triangle, aborting!\n");
			return false;
		}
	}
	return true;
}

/*
Solves the last 7 symbols of the combination by finding the barycentric coordinates of the desired vector,
and computing which triangle it falls in at each subdivision level.
*/
bool SolveViaInterpolation(Vec3 desired, Vec3 v0, Vec3 v1, Vec3 v2, s32 out_indices[SUBDIVISION_COUNT])
{
	// Subdivide the triangle 7 times, splitting by 8 each time.
	s32 subdivisions = SUBDIVISION_AMOUNT;
	for (s32 i = 1; i < SUBDIVISION_COUNT; ++i) subdivisions *= SUBDIVISION_AMOUNT;

	// Compute the barycentric coordinates of our desired point within the triangle.
	Vec2 bary = CartesianToBarycentric(desired, v0, v1, v2);
	printf("Computed Barycentric Coordinates: (%.15f, %.15f, %.15f)\n", bary.u, bary.v, 1.0 - bary.u - bary.v);
	if (bary.u + bary.v > 1.0)
	{
		printf("Desired point seems to lie outside the provided face, aborting!\n");
		return false;
	}

	// Convert to a 3D index, by scaling to the number of subdivisions and rounding down.
	IVec3 rounded_bary = IVec3((s32)(bary.u * subdivisions), (s32)(bary.v * subdivisions), (s32)((1.0 - bary.u - bary.v) * subdivisions));
	printf("Rounded down to nearest subdivision: (%d, %d, %d)\n", rounded_bary.x, rounded_bary.y, rounded_bary.z);

	IVec3 current_bary = rounded_bary;
	s32 current_divisions = subdivisions;
	s32 output_idx = SUBDIVISION_COUNT - 1;
	for (s32 i = 0; i < SUBDIVISION_COUNT; ++i)
	{
		Vec2 r0, r1, r2;
		double inc = 1.0 / current_divisions; // Increment amount.
		current_divisions /= 8; // Move out one subdivision level.

		// Half of the triangles are upside down, so we need to reorient/reorder the vertices.
		// If the indices add up to an even number, we know our triangle is upside down (don't ask me why this is true).
		if ((current_bary.x + current_bary.y + current_bary.z) % 2 == 0)
		{
			r0 = Vec2(current_bary.x + 1, current_bary.y + 1) * inc;
			r1 = Vec2(current_bary.x, current_bary.y + 1) * inc;
			r2 = Vec2(current_bary.x + 1, current_bary.y) * inc;
		}
		else
		{
			r0 = Vec2(current_bary.x, current_bary.y) * inc;
			r1 = Vec2(current_bary.x + 1, current_bary.y) * inc;
			r2 = Vec2(current_bary.x, current_bary.y + 1) * inc;
		}

		// Get the cartesian coordinates of the triangle vertices, just to raycast against as a sanity check.
		Vec3 out_v0 = BarycentricToCartesian(r0, v0, v1, v2);
		Vec3 out_v1 = BarycentricToCartesian(r1, v0, v1, v2);
		Vec3 out_v2 = BarycentricToCartesian(r2, v0, v1, v2);

		Vec3 intersection = {};
		Vec2 intersection_bary = {};
		bool does_intersect = RayTriangleIntersect(Normalize(desired), out_v0, out_v1, out_v2, &intersection, &intersection_bary.u, &intersection_bary.v);

		// Find our barycentric index at this subdivision level. The resulting subdivided triangle can lie outside our larger triangle.
		// If this happens, we can convert to the corresponding index in the next triangle, by adding 1 to each component and subtracting from 8.
		// Do not ask me why this works.
		IVec3 local_bary = IVec3(current_bary.x % SUBDIVISION_AMOUNT, current_bary.y % SUBDIVISION_AMOUNT, current_bary.z % SUBDIVISION_AMOUNT);
		if (local_bary.x + local_bary.y + local_bary.z > SUBDIVISION_AMOUNT) local_bary = Vec3(SUBDIVISION_AMOUNT) - Vec3((local_bary + IVec3(1, 1, 1)));
		printf("Triangle Indices: (%d, %d, %d) (Does Intersect? %s)\n", local_bary.x, local_bary.y, local_bary.z, does_intersect ? "Yes" : "No");
		current_bary /= SUBDIVISION_AMOUNT;

		// Convert our barycentric indices to a triangle index in our arbitrary scheme. We want to find the index of the "top" vertex.
		// In many cases, this vertex is shared between two triangles. By default we will use the second triangle in the list if this
		// happens, unless this is an upside down triangle, in which case we prefer the first.
		bool use_first = false;

		// For upside down triangles, set the "use first" flag, and offset accordingly.
		if ((local_bary.x + local_bary.y + local_bary.z) % 2 == 0)
		{
			local_bary.x += 1;
			local_bary.y += 1;
			use_first = true;
		}

		// Since this is a triangle, each level up has one fewer vertices than the previous.
		// To index in the Y axis, we add 9, then 8, then 7, etc.
		// For example, if our 2D vertex was (3, 4), then the linear index is (3) + (9 + 8 + 7 + 6).
		s32 idx = local_bary.x;
		s32 start = 9;
		for (s32 j = 0; j < local_bary.y; ++j)
		{
			idx += (start - j);
		}

		// Use our barycentric lookup table to find the triangle index using our linear vertex index,
		// accounting for the "use first" flag mentioned above.
		s32 tri1 = -1;
		s32 tri2 = -1;
		for (s32 j = 0; j < ARRAYCOUNT(bary_lut); ++j)
		{
			if (idx == bary_lut[j].x)
			{
				if (tri1 >= 0) tri2 = j;
				else tri1 = j;
			}
		}
		if (!use_first && tri1 >= 0 && tri2 >= 0) tri1 = tri2;

		if (output_idx >= 0) out_indices[output_idx--] = tri1;
		else
		{
			printf("We found too many triangle indices, something went wrong!\n");
			return false;
		}
	}

	return true;
}

/*
Solves the orientation of our ball in space given two known vectors from starmapping research.
Uses the resulting ball to compute the first symbol, and then uses the triangular face for that
symbol to compute the remaining 7, with two methods. You will need to build three mapping files yourself:

One is way to map each symbol to one vertex of an icosahedron, and two vertices of a dodecahedron.
Another maps each symbol to a triangle subdivided into 64 smaller ones, using the indexing scheme from ParseMapping2D().
The remaining file is just a list of vectors from starmapping research, and their corresponding symbol IDs.

I created the first two mappings using various paint programs, with Blender and UE4 to visualize and create the 3D map.
*/
s32 SolveRotation(s32 id1, s32 id2, Vec3 desired, const char* mapping_3d_path, const char* starmap_path, const char* mapping_2d_path)
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
	FILE* triangles_file = fopen(mapping_3d_path, "r");
	if (!triangles_file)
	{
		printf("Unable to open file %s\n", mapping_3d_path);
		return 1;
	}
	success = ParseTriangleTable(triangles_file, triangle_table);
	fclose(triangles_file);
	if (!success)
	{
		printf("Unable to parse triangle lookup table in file %s\n", mapping_3d_path);
		return 1;
	}

	s32 mapping_table[64] = {};
	FILE* mapping_file = fopen(mapping_2d_path, "r");
	if (!mapping_file)
	{
		printf("Unable to open file %s\n", mapping_2d_path);
		return 1;
	}
	success = ParseMapping2D(mapping_file, mapping_table);
	fclose(mapping_file);
	if (!success)
	{
		printf("Unable to parse 2d map lookup table in file %s\n", mapping_2d_path);
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
	Vec3 symbol_vectors[ARRAYCOUNT(triangle_table)] = {};
	for (s32 i = 0; i < ARRAYCOUNT(triangle_table); ++i)
	{
		Vec3 v_start = GetSymbolDirection(i + 1, triangle_table);
		Vec3 v = (diff_rot * Quat(Vec4(v_start, 0.0)) * Invert(diff_rot)).xyz;
		printf("%d,%.15f,%.15f,%.15f\n", i + 1, v.x, v.y, v.z);
		symbol_vectors[i] = v;
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
		Vec3 computed = symbol_vectors[id - 1];
		Vec3 starmap = v;
		double angle = Dot(computed, starmap) / (Length(computed) * Length(starmap));
		printf("%d,%.15f,%.15f,%.15f,%.15f,%.15f,%.15f,%.15f\n", id, angle, computed.x, computed.y, computed.z, starmap.x, starmap.y, starmap.z);
	}
	fclose(starmap_file);

	// Find the first symbol by raycasting our desired vector agaainst every triangular face in the ball.
	// Whichever face we hit is the first symbol in the combination!
	printf("\nFinding the first symbol...");
	s32 first_symbol = 0;
	Vec3 v0, v1, v2;
	for (s32 i = 1; i <= ARRAYCOUNT(triangle_table); ++i)
	{
		v0 = icosahedron[triangle_table[i - 1].x];
		v1 = dodecahedron[triangle_table[i - 1].y];
		v2 = dodecahedron[triangle_table[i - 1].z];

		Quat diff_rot_inv = Invert(diff_rot);
		v0 = (diff_rot * Quat(Vec4(v0, 0.0)) * diff_rot_inv).xyz;
		v1 = (diff_rot * Quat(Vec4(v1, 0.0)) * diff_rot_inv).xyz;
		v2 = (diff_rot * Quat(Vec4(v2, 0.0)) * diff_rot_inv).xyz;

		Vec3 intersection;
		double u, v;
		bool does_intersect = RayTriangleIntersect(Normalize(desired), v0, v1, v2, &intersection, &u, &v);

		if (does_intersect)
		{
			printf("Intersection found with face for symbol ID %d:\nVertex 1: (%f, %f, %f)\nVertex 2: (%f, %f, %f)\nVertex 3: (%f, %f, %f)\n", i, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z);
			printf("Intersection Point: (%f, %f, %f), U=%f, V=%f\n", intersection.x, intersection.y, intersection.z, u, v);
			if (first_symbol > 0)
			{
				printf("The destination vector intersects more than one symbol face, that isn't supposed to happen!\n");
				return 1;
			}
			first_symbol = i;
			break;
		}
	}

	if (!first_symbol)
	{
		printf("The destination vector doesn't intersect any symbol faces.\n");
		return 1;
	}

	// Solve the remaining symbols via raycasting and interpolation, just to double check our work.
	s32 output_raycasting[SUBDIVISION_COUNT] = {};
	s32 output_interpolation[SUBDIVISION_COUNT] = {};
	printf("\nAttempting to solve using raycasting and subdividing...\n");
	if (!SolveViaRaycast(desired, v0, v1, v2, triangle_table, output_raycasting)) return 1;
	printf("\nAttempting to solve by interpolating barycentric coordinates...\n");
	if (!SolveViaInterpolation(desired, v0, v1, v2, output_interpolation)) return 1;
	for (s32 i = 0; i < ARRAYCOUNT(output_raycasting); ++i) output_raycasting[i] = mapping_table[output_raycasting[i]];
	for (s32 i = 0; i < ARRAYCOUNT(output_interpolation); ++i) output_interpolation[i] = mapping_table[output_interpolation[i]];

	printf("\nSolution found by raycasting and subdividing: (%d, ", first_symbol);
	for (s32 i = 0; i < ARRAYCOUNT(output_raycasting) - 1; ++i) printf("%d, ", output_raycasting[i]);
	printf("%d)\n", output_raycasting[ARRAYCOUNT(output_raycasting) - 1]);

	printf("Solution found by interpolating barycentric coordinates: (%d, ", first_symbol);
	for (s32 i = 0; i < ARRAYCOUNT(output_interpolation) - 1; ++i) printf("%d, ", output_interpolation[i]);
	printf("%d)\n", output_interpolation[ARRAYCOUNT(output_interpolation) - 1]);
	
	return 0;
}