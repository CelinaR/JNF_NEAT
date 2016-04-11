#include "neural_network.h"
#include "gene_examinator.h"
#include <cmath>
#include <stdexcept>


NeuralNetwork::NeuralNetwork(unsigned int numberOfInputs, unsigned int numberOfOutputs):
		genes(numberOfInputs * numberOfOutputs),
		inputNeurons(numberOfInputs),
		outputNeurons(numberOfOutputs),
		outputs(numberOfOutputs)
{
	GenerateOnlyEssentialGenes();
	BuildNetworkFromGenes();
}

NeuralNetwork::NeuralNetwork(const std::vector<Gene> & genes):
    genes(genes)
{
    GenerateOnlyEssentialGenes();
    BuildNetworkFromGenes();
}

NeuralNetwork::NeuralNetwork(std::vector<Gene> && genes):
	genes(genes)
{
	ReadNumberOfInputsAndOutputsFromGenes();
	BuildNetworkFromGenes();
}

void NeuralNetwork::SetInputs(const std::vector<float> & inputs)
{
	if (inputNeurons.size() != inputs.size())
	{
		throw std::out_of_range("Number of inputs provided doesn't match genetic information");
	}
	for(size_t i = 0U; i < inputNeurons.size(); ++i){
		inputNeurons[i]->SetInput(inputs[i]);
	};
	areOutputsUpToDate = false;
}

const std::vector<float> & NeuralNetwork::GetOrCalculateOutputs()
{
	if (areOutputsUpToDate) {
		return outputs;
	}

	for(size_t i = 0U; i < outputs.size(); ++i){
		outputs[i] = outputNeurons[i]->RequestDataAndGetActionPotential();
	}

	areOutputsUpToDate = true;
	return outputs;
}

const std::vector<Gene> & NeuralNetwork::GetGenes() const {
	return genes;
}

void NeuralNetwork::BuildNetworkFromGenes() {
	DeleteAllNeurons();

	neurons.resize(GeneExaminator::GetNumberOfNeuronsInGenes(genes));
	for (const auto & gene : genes) {
		if (gene.isEnabled) {
			Neuron::IncomingConnection connection;
			connection.incoming = &neurons[gene.from];
			connection.weight = gene.weight;
			neurons[gene.to].AddConnection(std::move(connection));
		}
	}
	InterpretInputsAndOutputs();

	areOutputsUpToDate = false;
}

void NeuralNetwork::DeleteAllNeurons() {
	neurons.clear();
	for (auto in : inputNeurons) {
		in = nullptr;
	}
	for (auto out : outputNeurons) {
		out = nullptr;
	}
}

void NeuralNetwork::GenerateOnlyEssentialGenes() {
	if(genes.size() != inputNeurons.size() * outputNeurons.size()){
		throw std::out_of_range("Number of inputs provided doesn't match genetic information");
	}
	auto currentGene = &genes.front();
	for (auto in = 0U; in < inputNeurons.size(); ++in) {
		for (auto out = inputNeurons.size(); out < inputNeurons.size() + outputNeurons.size(); ++out){
			currentGene->from = in;
			currentGene->to = out;
			currentGene->isEnabled = true;

			++currentGene;
		}
	}
	areOutputsUpToDate = false;
}

void NeuralNetwork::InterpretInputsAndOutputs()
{
	for (auto i = 0U; i < inputNeurons.size(); i++) {
		inputNeurons[i] = &neurons[i];
	}
	for (auto i = 0U; i < outputNeurons.size(); i++) {
		outputNeurons[i] = &neurons[genes[i * outputNeurons.size()].to];
	}
}

void NeuralNetwork::ReadNumberOfInputsAndOutputsFromGenes() {
	auto numberOfInputs = 1U;
	auto numberOfOutputs = 1U;

	// TODO jnf
	// This is awful, rewrite it
	auto i = numberOfInputs;
	for (; i < genes.size(); ++i) {
		if (genes[i].from == genes[i - 1U].from + 1U) {
			numberOfInputs++;
		} else 
		if (genes[i].from != genes[i - 1U].from) {
			break;
		}
	}
	numberOfOutputs = genes[i - 1U].to - numberOfInputs;
	inputNeurons.resize(numberOfInputs);
	inputNeurons.resize(numberOfOutputs);
}













