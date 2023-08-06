Scripts and stuff that we used for Factorio Space Exploration. Spoilers, obviously :)

_I Still need to implement the final section of math once we get the 2D map created, but currently this code can solve the Interburbul puzzle, and translate all the symbols into direction vectors._

## Quick Writeup of how I figured out the last piece of the puzzle (I think):
Alright, I think I've worked it out. We've translated all the symbols into direction vectors, and we know what vector we want to find (I think). Now the question is: How do we dial the stargate thing to the vector we want? My first guess was that it just adds whatever 8 vectors you give it, but that doesn't seem right - something more complicated is going on.

One thing we know is that the pyramids have two diagrams each - A big disc and a small disc:

<img width="883" alt="pyramid_example" src="https://github.com/FrogBottom/SpaceExplorationStuff/assets/10749734/6edce2bb-823e-45da-9211-9d0169ee4ce1">

We used the big disc figure out which symbols are adjacent to each other on a ball, like constellations in the sky (technically a pentakis dodecahedron, but who cares):

![ball](https://github.com/FrogBottom/SpaceExplorationStuff/assets/10749734/f0820997-09af-4e7d-9e2c-834c8fe16342)

This is how we translated all the symbols into vectors. The small disc lets us figure out which symbols are adjacent to each other on some sort of 2D shape. I think it connects together into a big triangle (like the one below), made of a bunch of small triangles (one per symbol). So the thing to figure out is: How does putting all the symbols in a big triangle help us?

![tris](https://github.com/FrogBottom/SpaceExplorationStuff/assets/10749734/5e53bf3c-d811-47f3-9440-2cda962f4bb7)

A few other things I noticed:
* The order of symbols on the stargate matters, which makes me think that each symbol must be some operation that gets applied, one after another. Maybe the first symbol is the "starting" direction, and then each symbol after that modifies it somehow?
* The stargate has 64 symbols, and if you were to make a big triangle out of small triangles, the closest size for the big triangle would hold 64 small triangles. We only got 60 symbols from the pyramids though, which is enough to map the whole ball. So there are 4 extra symbols on the triangle.
  * Could be the three corners and the middle, I guess, for symmetry? Not sure. I think we will probably end up with the big triangle, 4 slots with no symbol, and 4 extra symbols.
* Some symbols can't be used in certain slots in the stargate, for some reason.

Update: Taking a look at the remaining four symbols, I think I'm right:

![Screenshot 2023-08-05 205719](https://github.com/FrogBottom/SpaceExplorationStuff/assets/10749734/5f11b213-adf6-44a6-82dd-9de74efd58b5)

To me, those four look a whole lot like "lower left of a triangle", "lower right of a triangle", "top of a triangle", and "middle of a triangle"...

###  Where to go from here...
At this point I was thinking about how I might encode stuff if I was designing a similar puzzle. The player has a bunch of vectors, how do you set it up so that they can be combined into a result, but where the order matters? Maybe each symbol is a smaller and smaller offset from the first vector? Then it would feel like you're dialing in closer and closer to your target, which is neat.

Also the little "interburbul" puzzle we did felt kinda, but not quite, related; It involved 3D vectors, but it was about dividing a quad into smaller ones, and interpolating to find the answer. Weird how the grid got more and more dense as we went. Hmm.

And then I googled "pyramid made of 60 triangles", to look for an image like the one above. One of the image captions was about Sierpinski triangles... They are fractals, you've probably seen one:

![sierpinski](https://github.com/FrogBottom/SpaceExplorationStuff/assets/10749734/fad91f40-c40c-43c9-be55-92e50d1612c0)

This gave me a rare feeling called "having an idea".

All the symbols correspond to faces on our big ball, and those faces are... triangles. So, hypothesis:
* The first symbol picks which triangular face of the ball we start with, using the map from the first diagram in all the pyramids.
* Take that triangle, and split it into 64 smaller ones.
* The second symbol picks one of the small triangles inside the big one, using the map from the second diagram in all the pyramids.
* Take that small triangle, and split it into 64 smaller ones.
* Repeat with all 8 symbols, zooming in more and more with each one.

So we are picking one triangular face of our ball, and subdividing it more and more and more to get closer and closer to our desired direction. If this is how it works, then we should expect the following:
* The resulting vector from the stargate should always be close-ish to whatever the first symbol in the combination is.
* If we enter one symbol and then seven of the "middle of the triangle" symbol, we should just get the symbol for that vector.
* If we enter one symbol and then seven of the other three "corner of the triangle" symbols, we should get a vector very close to the vertex of the face.

I went and checked some of this with the stargate, and it looks like this theory holds!

Now I just have to put together the 2D triangle map from all 60 pyramid screenshots, and then... find a way like, recursively iterate towards the desired vector using this weird fractal thing.
