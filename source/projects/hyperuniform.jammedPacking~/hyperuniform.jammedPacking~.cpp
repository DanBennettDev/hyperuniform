/// @file	
///	@ingroup 	CPG
/// @author		Dan Bennett
///	@license	Usage of this file and its contents is governed by the MIT License


/*
	TODO:
		Allow changing number of species
		Misses some triggers on fast rhythms / small spheres
*/


#include "c74_min.h"
#define SPECIES_LIST_SIZE 3
#define STORE_N_SPHERES 1

using namespace c74::min;


class jammedPacking : public object<jammedPacking>, public sample_operator<0, 3> {
private:


	struct softSphere {
		int speciesNo{ 0 };
		float diameter{ 2756.f };
		float softness{ 0.0f };
		float probability{ 0.f };
		INT64 marker{ 0 };
	};

	vector<softSphere> _lastNSpheres;
	vector<softSphere> _sphereSpecies;
	vector<float> _speciesAbundance;
	vector<float> _speciesActive;
	float _nextThreshold;

	vector<softSphere> _candidateList;
	INT64 _currMarker{ 0 };
	float _softnessExponent{ 2 };




public:
	bool m_initialized{ false };

	MIN_DESCRIPTION { "Rhythm generator based on the idea of hyperuniformity in the disordered jammed packing of soft spheres" };
	MIN_TAGS		{	"rhythm, generator"		};
	MIN_AUTHOR		{	"Daniel Bennett"			};
	MIN_RELATED		{	"phasor~" };

	jammedPacking(const atoms& args = {})
	{
		m_initialized = false;

		cout << "started" << endl;

		//srand((unsigned)time(NULL));
		srand(1);

		for (int i = 0; i < SPECIES_LIST_SIZE; i++) {
			_sphereSpecies.push_back(softSphere());
			_sphereSpecies[i].speciesNo = i;
			//_sphereSpecies[i].diameter = samplerate() / 16;
			_sphereSpecies[i].diameter = 2756;
			_speciesAbundance.push_back(1);
			_speciesActive.push_back(0);
		}

		_nextThreshold = getNextThreshold();

		for (int i = 0; i < SPECIES_LIST_SIZE; i++) {
			addObjectToCandidateList();
		}

		
		m_initialized = true;
	}

	~jammedPacking(){
		// object-specific tear-down code here
	}

	inlet<>			in	{ this, "(messages) input"};
	outlet<>		out1{ this, "pulse for voice 0", "signal" };
	outlet<>		out2{ this, "pulse for voice 1", "signal" };
	outlet<>		out3{ this, "pulse for voice 2", "signal" };



	samples<3> operator()()
	{
		_currMarker++;
		if (tryToPlaceObject()) {
			auto activeVoice = _lastNSpheres.back().speciesNo;
			_speciesActive[activeVoice] = _speciesActive[activeVoice] == 1 ? 0 : 1;
		}
		return {{ _speciesActive[0], _speciesActive[1], _speciesActive[2] }};
	}


	message<> setDiameter{ this, "setDiameter",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size() && (float)args[1] > 0) {
			float samples = ((float)args[1] * samplerate())/1000.f;
			_sphereSpecies[args[0]].diameter = samples;
		}
	return {};
	}
	};

	message<> setSoftness{ this, "setSoftness",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size()) {
			_sphereSpecies[args[0]].softness = (float)args[1];
		}
	return {};
	}
	};

	message<> setAbundance{ this, "setAbundance",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size()) {
			_speciesAbundance[args[0]] = args[1];
		}
	return {};
	}
	};

	message<> exp{ this, "exp",
		MIN_FUNCTION{
		if (args.size() >= 1) {
			setGlobalsoftnessExponent((float)args[0]);
		}
	return {};
	}
	};



	// sum the resistive force from one other sphere on this sphere
	float placementResistance(float diameterA, float diameterB, float softnessA, float softnessB,
		float distance, float softnessExp)
	{
		// calc amount of "give" in the objects' fields
		float giveA = diameterA * softnessA;
		float giveB = diameterB * softnessB;

		float compression = (diameterA + diameterB) - distance;
		if (compression <= 0) {
			return 0;
		}
		// if placement would cause greater compression than the "give" of the spheres, resistance is 100%
		if (compression > giveA + giveB) {
			return 1;
		}
		// share the compression between the two spheres
		// note: incredibly crude! Not realistic! Cheap! Probably good enough for our purposes!
		float compressionRatio = giveA / (giveA+giveB);
		float cA = compressionRatio * compression;
		float cB = (1.f - compressionRatio) * compression;
		// calculate total resistance to compression
		return  (pow(cA / giveA, softnessExp) + pow(cB/giveA, softnessExp))/2.0f;
	}




	// sum resistance on current placement from all spheres in scope
	float sumPlacementResistance(float thisDiameter, float thissoftness)
	{
		float resistance = 0;
		for (auto s : _lastNSpheres) {
			INT64 distance = _currMarker - s.marker;
			resistance += placementResistance(thisDiameter, s.diameter, thissoftness,
				s.softness, (float)distance, _softnessExponent);
		}
		return resistance > 1 ? 1 : resistance;
	}


	bool tryToPlaceObject() {
		// set probability thresholds on each of the candidate objects
		for (auto &sphere : _candidateList) {
			sphere.probability = 1.0f - sumPlacementResistance(sphere.diameter, sphere.softness);
		}

		//float result = ((float)rand() / (float)RAND_MAX) * _candidateList.size();
		float probSum = 0;
		for (int i = 0; i<_candidateList.size(); i++) {
			if (_candidateList[i].probability != 0 && _nextThreshold < (_candidateList[i].probability + probSum)) {
				// remove object
				softSphere s = _candidateList[i];
				if (_lastNSpheres.size() > 0) {
					// DEBUGGING
					INT64 distance = _currMarker - _lastNSpheres.back().marker;
					distance = distance;
				}
				_candidateList.erase(_candidateList.begin() + i);

				// replace object in candidate list
				addObjectToCandidateList();

				s.marker = _currMarker;
				_lastNSpheres.push_back(s);
				if (_lastNSpheres.size() > STORE_N_SPHERES) {
					_lastNSpheres.erase(_lastNSpheres.begin());
				}

				_nextThreshold = getNextThreshold();
				return true;
			}
			probSum += _candidateList[i].probability;
		}
		return false;

	}

	bool tick() {
		_currMarker++;
		bool newEvent = tryToPlaceObject();
		return newEvent;
	}

	// when an object is placed, replenish the candidate list (from object species list)
	// try different rules out here - e.g. random / ordered
	// have a queue of n candidates & pick from this list
	// candidates added to queue by some kind of availability function 
	//	- if we've already used 5 of b, then it becomes less likely to be added
	// or from a user-defined sequence

	void addObjectToCandidateList() {
		float total = 0;
		for (int i = 0; i < _speciesAbundance.size(); i++) {
			total += _speciesAbundance[i];
		}
		float result = ((float)rand() / (float)RAND_MAX) * total;

		float cum = 0;
		for (int i = 0; i < _speciesAbundance.size(); i++) {
			if (result < cum + _speciesAbundance[i]) {
				_candidateList.push_back(_sphereSpecies[i]);
				return;
			}
			cum += _speciesAbundance[i];
		}
	}

	void removeSpecies(int speciesNo) {
		if (speciesNo<_sphereSpecies.size() && _sphereSpecies.size() != 1) {
			_sphereSpecies.erase(_sphereSpecies.begin() + speciesNo);
			for (int i = 0; i < _sphereSpecies.size(); i++) {
				_sphereSpecies[i].speciesNo = i;
			}
		}
	}

	int addSpecies(float diameter, float softness) {
		if (diameter > 0 && softness > 0 && softness < 1) {
			softSphere s;
			s.diameter = diameter;
			s.softness = softness;
			_sphereSpecies.push_back(s);
			s.speciesNo = _sphereSpecies.size();
			return _sphereSpecies.size() - 1;
		}
		return 0;
	}

	void setGlobalsoftnessExponent(float softnessExp) {
		softnessExp = softnessExp < 0 ? 0 : softnessExp;
		softnessExp = softnessExp > 1 ? 1 : softnessExp;

		_softnessExponent = softnessExp;
	}

	float getNextThreshold() {
		return ((float)rand() / (float)RAND_MAX) * _candidateList.size();
	}

};

MIN_EXTERNAL(jammedPacking);
