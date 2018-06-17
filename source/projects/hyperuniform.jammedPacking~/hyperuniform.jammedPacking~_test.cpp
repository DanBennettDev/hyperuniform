// Copyright (c) 2016, Cycling '74
// Timothy Place
// Usage of this file and its contents is governed by the MIT License

#include "c74_min_unittest.h"		// required unit test header
#include "hyperuniform.jammedPacking~.cpp"	// need the source of our object so that we can access it

// Unit tests are written using the Catch framework as described at
// https://github.com/philsquared/Catch/blob/master/docs/tutorial.md

TEST_CASE("calculates a vector when provided valid values") {
	ext_main(nullptr);	// every unit test must call ext_main() once to configure the class

	// create an instance of our object
	test_wrapper<jammedPacking> an_instance;
	jammedPacking& my_object = an_instance;

	// create an impulse buffer to process
	const int		buffersize = 256;
	const int		channels = 256;

	double** samplesIn = new double*[channels];
	double** samplesOut = new double*[channels];

	for (int i = 0; i < channels; ++i) {
		samplesIn[i] = new double[buffersize];
		samplesOut[i] = new double[buffersize];
	}

	audio_bundle in(samplesIn, 3, 256);
	audio_bundle out(samplesOut, 3, 256);

	my_object(in, out);


	REQUIRE ( true );
}


