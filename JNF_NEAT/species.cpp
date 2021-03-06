#include "species.h"
#include <algorithm>
#include <cstdlib>

using namespace JNF_NEAT;
using namespace std;

Species::Species(const Organism& representative) :
parameters(representative.GetTrainingParameters()) {
	population.push_back(new Organism(representative));
	ElectRepresentative();
}

Species::Species(Organism&& representative) :
parameters(representative.GetTrainingParameters()) {
	population.push_back(new Organism(move(representative)));
	ElectRepresentative();
}

Species::~Species() {
    for (auto* organism : population) {
        delete organism;
        organism = nullptr;
    }
}

void Species::AddOrganism(const Organism& organism) {
	population.push_back(new Organism(organism));
	ElectRepresentative();
	isSortedByFitness = false;
    SetPopulationsFitnessModifier();
}

void Species::AddOrganism(Organism&& organism) {
	population.push_back(new Organism(move(organism)));
	ElectRepresentative();
	isSortedByFitness = false;
	SetPopulationsFitnessModifier();
}

void Species::AnalyzeAndClearPopulation() {
	const auto currentBestFitness = GetFittestOrganism().GetOrCalculateRawFitness();
	if (currentBestFitness > fitnessHighscore) {
		fitnessHighscore = currentBestFitness;
		numberOfStagnantGenerations = 0;
	} else {
		numberOfStagnantGenerations++;
	}

    for (auto* organism : population) {
        delete organism;
        organism = nullptr;
    }
    population.clear(); 
    isSortedByFitness = false;	
}


bool Species::IsCompatible(const Genome& genome) const {
	auto distanceToSpecies = representative->GetGenome().GetGeneticalDistanceFrom(genome);
	return !IsAboveCompatibilityThreshold(distanceToSpecies);
}

void Species::SetPopulationsFitnessModifier() {
	double fitnessModifier = 1.0 / (double)population.size();
	for (auto* organism : population){
		organism->SetFitnessModifier(fitnessModifier);
	}
}

void Species::ElectRepresentative() {
	if (!population.empty()) {
		SelectRandomRepresentative();
		//SelectFittestOrganismAsRepresentative();
	}
}

void Species::SelectRandomRepresentative() {
	auto randomMember = rand() % population.size();
	if (representative == nullptr) {
		representative = make_unique<Organism>(*population[randomMember]);
    } else {
        *representative = *population[randomMember];
    }
}

void Species::SelectFittestOrganismAsRepresentative() {
	if (representative == nullptr) {
		representative = make_unique<Organism>(GetFittestOrganism());
	} else {
		*representative = GetFittestOrganism();
	}
}

void Species::LetPopulationLive() {
	for (auto* organism : population){
		organism->Update();
	}
	isSortedByFitness = false;
}

void Species::ResetToTeachableState() {
	for (auto* organism : population){
		organism->Reset();
	}
}

Organism& Species::GetFittestOrganism() {
    if (population.empty()) {
        return *representative;
    }
	if (!isSortedByFitness) {
		auto CompareOrganisms = [&](Organism* lhs, Organism* rhs) {
			return lhs->GetOrCalculateFitness() > rhs->GetOrCalculateFitness();
		};
		sort(population.begin(), population.end(), CompareOrganisms);
		isSortedByFitness = true;
	}
	return *population.front();
}

Species& Species::operator=(Species&& other) {
	population = move(other.population);
	representative = move(other.representative);
	isSortedByFitness = move(other.isSortedByFitness);
	numberOfStagnantGenerations = move(other.numberOfStagnantGenerations);
	fitnessHighscore = move(other.fitnessHighscore);
	return *this;
}

Organism& Species::GetOrganismToBreed() {
	// TODO jnf: Switch to stochastic universal sampling
    if (population.empty()) {
        return *representative;
    }
	double totalPopulationFitness = 0.0;
	for (auto* organism : population) {
		totalPopulationFitness += organism->GetOrCalculateFitness();
	}
	if (totalPopulationFitness == 0) {
		return *population[rand() % population.size()];
	}
	double chance = 0.0;
	auto GetChanceForOrganism = [&chance, &totalPopulationFitness](Organism& organism) {
		return chance + (organism.GetOrCalculateFitness() / totalPopulationFitness);
	};

    while (true) {
        for (auto* organism : population) {
            double randNum = (double)(rand() % 10'000) / 9'999.0;;
            chance = GetChanceForOrganism(*organism);
            if (randNum < chance) {
                return *organism;
            }
        }
    }
}