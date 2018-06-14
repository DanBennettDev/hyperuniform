

// rhythm generator inspired by the notion of hyperuniformity in the disordered, jammed packing of soft spheres
#include <time.h>
#include <stdlib.h>

class softSpheres {

	struct object {
		float diameter;
		float elasticity;

		object(){
			diameter = 1.f;
			elasticity = 0.f;
		}
	}

	vector<float> _lastNObjects;
	vector<object> _objectSpecies;
	vector<object> _candidateList;
	int64 _currMarker {0};
	float _elasticityExponent{2};



	void softSpheres(){

		srand (time(NULL))
		_objectSpecies.push_back(object());
	}


	// sum the resistive force from one other sphere on this sphere
	float placementResistance(float diameterA, float diameterB, float elasticityA, float elasticityB,
										float distance, float elasticityExp)
	{
		// calc amount of "give" in the objects' fields
		float giveA = diameterA * elasticityA;
		float giveB = diameterB * elasticityB;

		// 
		float diameter1, diameter2, give1, give2, comp1, comp2;
		if((diameterA * elasticityA) > (diameterB * elasticityB)){
			diameter1 = diameterA;
			diameter2 = diameterB;
			give1 = giveA;
			give2 = giveB
		} else {
			diameter1 = diameterB;
			diameter2 = diameterA;
			give1 = giveB;
			give2 = giveA;
		}

		float compression = (diameter1 + diameter2) - distance;
		if(compression < 0){
			return 0;
		}
		// if placement would cause greater compression than the "give" of the spheres, resistance is 100%
		if (compression > give1  + give2){
			return 1;
		}

		float compressionRatio = pow(e1, elasticityExp) / pow(e2, elasticityExp);
		float c1 = compressionRatio * compression; 

		return  2 * pow(c1/e1, elasticityExp);
	}


	// sum resistance on current placement from all spheres in scope
	float sumPlacementResistance(float thisDiameter, float thisElasticity)
	{
		float resistance = 0;
		for(auto object : _lastNObjects){
			distance = currMarker - object.marker;
			resistance += placementResistance(thisDiameter, object.diameter, thisElasticity, 
													object.elasticity, distance, _elasticityExponent)
		}
		return resistance > 1 ? 1 : resistance;
	}

	object tick(){
		currMarker++;
		return tryToPlaceObject();
	}



	object tryToPlaceObject(){
		// set probability thresholds on each of the candidate objects
		for(auto object: _candidateList){
			object.probability = 1.0f - sumPlacementResistance(object.diameter, object.elasticity);
		}

		float result = (rand() / RAND_MAX) * _candidateList.size();
		float probSum = 0;
		for(int i=0; i<_candidateList.size(); i++){
			if(_candidateList[i].probability != 0 && result < (_candidateList[i].probability + probSum)){
				// remove object
				auto o = _candidateList[i];
				_candidateList.erase(_candidateList.begin() + i);

				// replace object in candidate list
				addObjectToCandidateList();
				

				return object;
			}
			probSum +=object.probability;
		}
		return null;
		
	}


	// when an object is placed, replenish the candidate list (from object species list)
	// try different rules out here - e.g. random / ordered
		// have a queue of n candidates & pick from this list
		// candidates added to queue by some kind of availability function 
		//	- if we've already used 5 of b, then it becomes less likely to be added
		// or from a user-defined sequence

	void addObjectToCandidateList(){
		// simple at present - random object from species list
		int i = rand() % _objectSpecies.size();
		_candidateList.push_back(_objectSpecies[i]);
	}

	void removeSpecies(int speciesNo){
		if(speciesNo<_objectSpecies.size() && _objectSpecies.size()!=1){
			_objectSpecies.erase(_objectSpecies.begin() + speciesNo);
		}
	}

	int addSpecies(float diameter, float elasticity){
		if(diameter > 0 && elasticity > 0 && elasticity < 1){
			object s;
			s[voice].diameter = diameter;
			s[voice].elasticity = elasticity;
			_objectSpecies.push_back(s);
			return _objectSpecies.size()-1;
		}
		return 0;
	}

	void setGlobalElasticityExponent(float elasticityExp){
		elasticityExp = elasticityExp < 0 ? 0 : elasticityExp;
		elasticityExp = elasticityExp > 1 ? 1 : elasticityExp;

		_elasticityExponent = elasticityExp;
	}



}