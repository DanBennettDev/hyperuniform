/// @file	
///	@ingroup 	CPG
/// @author		Dan Bennett
///	@license	Usage of this file and its contents is governed by the MIT License


/*
	TODO:
		make each voice directly triggerable
		send triggers not inversion (width controllable by user)
		velocity from (inverse)resistance
*/


#include "c74_min.h"
//#define SPECIES_LIST_SIZE 3
#define STORE_N_SPHERES 1
#define DEFAULTWIDTH 2756.f
#define MAXVOICES 16

using namespace c74::min;


class jammedPacking : public object<jammedPacking>, public vector_operator<> {
private:


	struct softSphere {
		int speciesNo{ 0 };
		float diameter{ DEFAULTWIDTH };
		float softness{ 0.0f };
		float probability{ 0.f };
		INT64 marker{ 0 };
	};

	vector<softSphere> _lastNSpheres;
	vector<softSphere> _sphereSpecies;
	vector<float> _speciesAbundance;
	vector<float> _speciesActive;
	float _pFire;

	vector<softSphere> _candidateList;
	INT64 _currMarker{ 0 };
	float _softnessExponent{ 2 };
	int counter{ 0 };



public:
	bool _initialized{ false };

	MIN_DESCRIPTION { "Rhythm generator based on the idea of hyperuniformity in the disordered jammed packing of soft spheres" };
	MIN_TAGS		{	"rhythm, generator"		};
	MIN_AUTHOR		{	"Daniel Bennett"			};
	MIN_RELATED		{	"phasor~" };

	jammedPacking(const atoms& args = {})
	{
		_initialized = false;

		cout << "started" << endl;

		//srand((unsigned)time(NULL));
		srand(1);

		int voices = args.size() >= 1 ? (int)args[0] : 3;
		voices = voices < 1 ? 1 : voices;
		voices = voices > MAXVOICES ? MAXVOICES : voices;

		// set up nodes and ins/outs for them
		for (int voice = 0; voice < voices; ++voice) {
			_sphereSpecies.push_back(softSphere());
			_sphereSpecies[voice].speciesNo = voice;
			_sphereSpecies[voice].diameter = DEFAULTWIDTH;
			_speciesAbundance.push_back(1);
			_speciesActive.push_back(0);

			_ins.push_back(std::make_unique<inlet<>>(this, "(signal) freq input " + voice));
			_outs.push_back(std::make_unique<outlet<>>(this, "(signal) signal output " + voice, "signal"));
		}

		_pFire = (float)rand() / (float)RAND_MAX;

		// this needs to be done after all the species have been added
		for (int i = 0; i < voices; i++) {
			addObjectToCandidateList();
		}

		_initialized = true;
	}

	~jammedPacking(){
		// object-specific tear-down code here
	}

	//inlet<>			in	{ this, "(messages) input"};
	//outlet<>		out1{ this, "pulse for voice 0", "signal" };
	//outlet<>		out2{ this, "pulse for voice 1", "signal" };
	//outlet<>		out3{ this, "pulse for voice 2", "signal" };

	vector< unique_ptr<inlet<>> >			_ins;				///< this object's ins
	vector< unique_ptr<outlet<>> >			_outs;				///< this object's outs

	//samples<3> operator()()
	//{
	//	_currMarker++;
	//	if (tryToPlaceObject()) {
	//		auto activeVoice = _lastNSpheres.back().speciesNo;
	//		_speciesActive[activeVoice] = _speciesActive[activeVoice] > 0.5 ? 0 : 1;
	//		auto expected = _currMarker / (DEFAULTWIDTH*2);
	//		auto received = counter;
	//		counter++;
	//	}
	//	return {{ _speciesActive[0], _speciesActive[1], _speciesActive[2] }};
	//}


	void operator()(audio_bundle input, audio_bundle output)
	{
		if (_initialized) {
			// For each frame in the vector calc each channel
			for (auto frame = 0; frame<input.frame_count(); ++frame) {
				_currMarker++;
				if (tryToPlaceObject()) {
					auto activeVoice = _lastNSpheres.back().speciesNo;
					_speciesActive[activeVoice] = _speciesActive[activeVoice] > 0.5 ? 0 : 1;
					auto expected = _currMarker / (DEFAULTWIDTH * 2);
					auto received = counter;
					counter++;
				}

				for (int channel = 0; channel < _sphereSpecies.size(); channel++) {
						output.samples(channel)[frame] = _speciesActive[channel];
					}
				}
			}

	}



	message<> bang{ this, "bang",
		MIN_FUNCTION{
			report();
	return {};
	}
	};

	message<> setDiameter{ this, "setDiameter",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size() && (float)args[1] > 0) {
			float samples = ((float)args[1] * samplerate())/1000.f;
			_sphereSpecies[args[0]].diameter = samples;
			updateCandidates();
		}
	return {};
	}
	};

	message<> setSoftness{ this, "setSoftness",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size()) {
			float s = (float)args[1];
			s > 0 ? 0 : s;
			s < 1 ? 1 : s;
			_sphereSpecies[args[0]].softness = s;
			updateCandidates();
		}
	return {};
	}
	};

	message<> setAbundance{ this, "setAbundance",
		MIN_FUNCTION{
		if (args.size() >= 2 && (int)args[0] < _sphereSpecies.size()) {
			float a = args[1];
			a > 0 ? 0 : a;
			a < 1 ? 1 : a;
			_speciesAbundance[args[0]] = a;
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
		// set probabilities on each of the candidate objects
		float pMax = 0, pSum=0;
		for (auto &sphere : _candidateList) {
			sphere.probability = 1.0f - sumPlacementResistance(sphere.diameter, sphere.softness);
			pMax = sphere.probability > pMax ? sphere.probability : sphere.probability;
			pSum += sphere.probability;
		}
		if (_pFire < pMax) {
			//weighted random voice selection
			float pSelect = pSum * _pFire;
			float pCum=0;
			for (int i = 0; i<_candidateList.size(); i++) {
				if (_candidateList[i].probability != 0 && pSelect < (_candidateList[i].probability + pCum)) {
					// remove object and replace
					softSphere s = _candidateList[i];
					_candidateList.erase(_candidateList.begin() + i);
					addObjectToCandidateList();

					s.marker = _currMarker;
					_lastNSpheres.push_back(s);
					if (_lastNSpheres.size() > STORE_N_SPHERES) {
						_lastNSpheres.erase(_lastNSpheres.begin());
					}
				}
				pCum += _candidateList[i].probability;
			}

			_pFire = (float)rand() / (float)RAND_MAX;
			return true;
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


	void setGlobalsoftnessExponent(float softnessExp) {
		softnessExp = softnessExp < 0 ? 0 : softnessExp;
		softnessExp = softnessExp > 1 ? 1 : softnessExp;

		_softnessExponent = softnessExp;
	}

	float getNextThreshold() {
		return ((float)rand() / (float)RAND_MAX); // *_candidateList.size();
	}

	void updateCandidates() {
		for (auto &candidate : _candidateList) {
			for (auto species : _sphereSpecies) {
				if (candidate.speciesNo = species.speciesNo) {
					candidate.diameter = species.diameter;
					candidate.softness = species.softness;
				}
			}
		}
	}

	void report() {
		cout << "diameters: "
			+ std::to_string(_sphereSpecies[0].diameter) + " "
			+ std::to_string(_sphereSpecies[1].diameter) + " "
			+ std::to_string(_sphereSpecies[2].diameter) << endl;
		cout << "softness: "
			+ std::to_string(_sphereSpecies[0].softness) + " "
			+ std::to_string(_sphereSpecies[1].softness) + " "
			+ std::to_string(_sphereSpecies[2].softness) << endl;
		cout << "abundances: "
			+ std::to_string(_speciesAbundance[0]) + " "
			+ std::to_string(_speciesAbundance[1]) + " "
			+ std::to_string(_speciesAbundance[2]) << endl;

		cout << "diameters: "
			+ std::to_string(_candidateList[0].diameter) + " "
			+ std::to_string(_candidateList[1].diameter) + " "
			+ std::to_string(_candidateList[2].diameter) << endl;

		cout << "lastOne: "
			+ std::to_string(_lastNSpheres.back().diameter) + " ("
			+ std::to_string(_lastNSpheres.size()) + ") "<< endl;
	}

};

MIN_EXTERNAL(jammedPacking);
