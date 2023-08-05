#include "Core.h"

//Inputs for one interburbul puzzle.
struct Burb
{
	s32 grid_size;
	IVec2 desired;
	Vec3 top_left;
	Vec3 top_right;
	Vec3 bottom_left;

	Vec3 Interburbulate();
};

/*
Computes the puzzle solution. I'm sure there is a fancy way to do this with a single matrix multiply or something,
but this just manually figures out two basis vectors, finds the square size in each axis, and then offsets from the
top left by the desired number of squares.
*/
Vec3 Burb::Interburbulate()
{
	// Normalized basis vectors.
	Vec3 x_axis = Normalize(top_right - top_left);
	Vec3 y_axis = Normalize(bottom_left - top_left);

	// Grid square size in each axis.
	Vec2 square_size = Vec2(Length(top_right - top_left), Length(bottom_left - top_left)) / (double)grid_size;

	// Starting at the top left, move the correct number of squares along each axis (offsetting by half a square).
	Vec3 x_result = x_axis * (square_size.x * (desired.x - 0.5));
	Vec3 y_result = y_axis * (square_size.y * (desired.y - 0.5));
	return top_left + x_result + y_result;
}

/*
Calls fscanf to try and parse a line from a text file for interburbul.

Returns the number of elements successfully parsed.
If not all elements could be parsed, the output will not be modified.
*/
static s32 ParseBurb(FILE* f, Burb* burb)
{
	s32 s, dx, dy;
	Vec3 tl, tr, bl;
	s32 count = fscanf(f, "%d,%d,%d,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", &s, &dx, &dy, &tl.x, &tl.y, &tl.z, &tr.x, &tr.y, &tr.z, &bl.x, &bl.y, &bl.z);
	if (count == 12) *burb = {s, {dx, dy}, tl, tr, bl};
	return count;
}

/*
Tries to compute the solution to interburbul, reading the puzzles from a CSV file.
Each row of the CSV should contain a puzzle (other than the first, which can be a header).
Rows should be formatted as:
Grid Size,Desired X, Desired Y, Top Left X, Top Left Y, Top Left Z, Top Right X, Top Right Y, Top Right Z, Bottom Left X, Bottom Left Y, Bottom Left Z

Returns 0 if successful, or 1 if an error occured when reading/parsing the file.
*/
s32 RunInterburbul(const char* file_path)
{
	FILE* f = fopen(file_path, "r");
	if (!f)
	{
		printf("Unable to open file %s\n", file_path);
		fclose(f);
		return 1;
	}
	
	printf("Index,X,Y,Z\n");
	s32 parsed_count = 0;
	Burb burb = {};

	// The first line can optionally be a header, which we will skip.
	// Otherwise rewind to the start of the file.
	if (fscanf(f, "%*d,%*d,%*d,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf,%*lf\n") == 0) fscanf(f, "%*[^\n]\n");
	else fseek(f, 0, SEEK_SET);

	s32 fields_parsed = 0;
	while ((fields_parsed = ParseBurb(f, &burb)) == 12)
	{
		Vec3 result = burb.Interburbulate();
		printf("%d,%lf,%lf,%lf\n", parsed_count, result.x, result.y, result.z);
		++parsed_count;
	}
	fclose(f);

	if (fields_parsed != 0 && fields_parsed != EOF)
	{
		printf("Unable to parse burb at index %d, is the line formatted correctly?\n", parsed_count);
		return 1;
	}
	else return 0;

}
