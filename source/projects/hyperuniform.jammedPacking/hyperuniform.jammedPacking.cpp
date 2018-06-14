/// @file	
///	@ingroup 	CPG
/// @author		Dan Bennett
///	@license	Usage of this file and its contents is governed by the MIT License


/*
	TODO:
		Allow changing number of species
		make signal rate
*/


#include "c74_min.h"
#define CANDIDATE_LIST_SIZE 16
#define SPECIES_LIST_SIZE 3

#define STORE_N_SPHERES 16

using namespace c74::min;


class jammedPacking : public object<jammedPacking> {
private:


	struct softSphere {
		int speciesNo{ 0 };
		float diameter{ 8.f };
		float softness{ 0.f };
		float probability{ 0.f };
		INT64 marker{ 0 };
	};

	vector<softSphere> _lastNSpheres;
	vector<softSphere> _sphereSpecies;
	vector<float> _speciesAbundance;
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

		srand((unsigned)time(NULL));

		for (int i = 0; i < SPECIES_LIST_SIZE; i++) {
			_sphereSpecies.push_back(softSphere());
			_sphereSpecies[i].speciesNo = i;
			_speciesAbundance.push_back(1);
		}

		

		for (int i = 0; i < CANDIDATE_LIST_SIZE; i++) {
			addObjectToCandidateList();
		}


		m_initialized = true;
	}

	~jammedPacking(){
		// object-specific tear-down code here
	}

	inlet<>			in	{ this, "(messages) input"};

	outlet<>		out	{ this, "(voice number) ramp wave", "int" };




	message<> bang { this, "bang", "metro tick in",
		MIN_FUNCTION {
			if (m_initialized) {
				cout << "bang";
				if (tick()) {
					out.send(_lastNSpheres.back().speciesNo);
				}

			}
			return {};
		}
	};


	message<> remove{ this, "remove",
		MIN_FUNCTION{
		if (args.size() >= 1) {
			removeSpecies((int)args[0]);
		}
	return {};
	}
	};

	message<> setDiameter{ this, "setDiameter",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size()) {
			_sphereSpecies[args[0]].diameter = args[1];
		}
	return {};
	}
	};

	message<> setSoftness{ this, "setSoftness",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size()) {
			_sphereSpecies[args[0]].softness = args[1];
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

		// 
		float diameter1, diameter2, give1, give2;
		if ((diameterA * softnessA) > (diameterB * softnessB)) {
			diameter1 = diameterA;
			diameter2 = diameterB;
			give1 = giveA;
			give2 = giveB;
		}
		else {
			diameter1 = diameterB;
			diameter2 = diameterA;
			give1 = giveB;
			give2 = giveA;
		}

		float compression = (diameter1 + diameter2) - distance;
		if (compression < 0) {
			return 0;
		}
		// if placement would cause greater compression than the "give" of the spheres, resistance is 100%
		if (compression > give1 + give2) {
			return 1;
		}

		float compressionRatio = pow(give1, softnessExp) / pow(give2, softnessExp);
		float c1 = compressionRatio * compression;

		return  2 * pow(c1 / give1, softnessExp);
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

	bool tick() {
		_currMarker++;
		bool newEvent = tryToPlaceObject();
		return newEvent;
	}



	bool tryToPlaceObject() {
		// set probability thresholds on each of the candidate objects
		for (auto &sphere : _candidateList) {
			sphere.probability = 1.0f - sumPlacementResistance(sphere.diameter, sphere.softness);
		}

		float result = ((float)rand() / (float)RAND_MAX) * _candidateList.size();
		float probSum = 0;
		for (int i = 0; i<_candidateList.size(); i++) {
			if (_candidateList[i].probability != 0 && result < (_candidateList[i].probability + probSum)) {
				// remove object
				softSphere s = _candidateList[i];
				_candidateList.erase(_candidateList.begin() + i);

				// replace object in candidate list
				addObjectToCandidateList();

				s.marker = _currMarker;
				_lastNSpheres.push_back(s);
				if (_lastNSpheres.size() > STORE_N_SPHERES) {
					_lastNSpheres.erase(_lastNSpheres.begin());
				}
				return true;
			}
			probSum += _candidateList[i].probability;
		}
		return false;

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
			if (total < cum + _speciesAbundance[i]) {
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


};

MIN_EXTERNAL(jammedPacking);
