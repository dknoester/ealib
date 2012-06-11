/* nn.cpp
 * 
 * This file is part of EALib.
 * 
 * Copyright 2012 David B. Knoester.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <vector>
#include <iterator>
#include <numeric>
#include <ctime>
#include <cmath>

#include <nn/feed_forward.h>
#include <nn/back_propagation.h>
#include <nn/layout.h>

using namespace nn;

double inputs[][2] = {
{ 0.72, 0.82 }, { 0.91, -0.69 }, { 0.46, 0.80 },
{ 0.03, 0.93 }, { 0.12, 0.25 }, { 0.96, 0.47 },
{ 0.79, -0.75 }, { 0.46, 0.98 }, { 0.66, 0.24 },
{ 0.72, -0.15 }, { 0.35, 0.01 }, { -0.16, 0.84 },
{ -0.04, 0.68 }, { -0.11, 0.10 }, { 0.31, -0.96 },
{ 0.00, -0.26 }, { -0.43, -0.65 }, { 0.57, -0.97 },
{ -0.47, -0.03 }, { -0.72, -0.64 }, { -0.57, 0.15 },
{ -0.25, -0.43 }, { 0.47, -0.88 }, { -0.12, -0.90 },
{ -0.58, 0.62 }, { -0.48, 0.05 }, { -0.79, -0.92 },
{ -0.42, -0.09 }, { -0.76, 0.65 }, { -0.77, -0.76 } };

// Load sample output patterns.
double outputs[] = {
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

double data[][3] = {
{0.10, 0.03, 0},
{0.11, 0.11, 0},
{0.11, 0.82, 0},
{0.13, 0.17, 0},
{0.20, 0.81, 0},
{0.21, 0.57, 1},
{0.25, 0.52, 1},
{0.26, 0.48, 1},
{0.28, 0.17, 1},
{0.28, 0.45, 1},
{0.37, 0.28, 1},
{0.41, 0.92, 0},
{0.43, 0.04, 1},
{0.44, 0.55, 1},
{0.47, 0.84, 0},
{0.50, 0.36, 1},
{0.51, 0.96, 0},
{0.56, 0.62, 1},
{0.65, 0.01, 1},
{0.67, 0.50, 1},
{0.73, 0.05, 1},
{0.73, 0.90, 0},
{0.73, 0.99, 0},
{0.78, 0.01, 1},
{0.83, 0.62, 0},
{0.86, 0.42, 1},
{0.86, 0.91, 0},
{0.89, 0.12, 1},
{0.95, 0.15, 1},
{0.98, 0.73, 0}};

double xord[][3] = {
{0,0,0},
{0,1,1},
{1,0,1},
{1,1,0}};

double oneone[][3] = {
{1,1},
{0,0}};

template <typename NeuralNetwork>
void train(NeuralNetwork& nn, std::size_t n) {
	for(std::size_t i=0; i<n; ++i) {
		double err=0.0;
		for(int j=0; j<4; ++j) {
			typename NeuralNetwork::neuron_map_type im, em;
			im[nn.input(0)] = data[j][0];
			im[nn.input(1)] = data[j][1];
			em[nn.output(0)] = data[j][2];
//			im[nn.input(0)] = inputs[j][0];
//			im[nn.input(1)] = inputs[j][1];
//			em[nn.output(0)] = outputs[j];
//			im[nn.input(0)] = xord[j][0];
//			im[nn.input(1)] = xord[j][1];
//			em[nn.output(0)] = xord[j][2];
//			im[nn.input(0)] = oneone[j][0];
//			em[nn.output(0)] = oneone[j][1];

			err += back_propagate(nn, im, em);
		}
		std::cout << i << " " << err << std::endl;
	}
}


int main(int argc, char * const argv[]) {
	feed_forward_neural_network nn;
	std::size_t layers[] = {2, 2, 1};
	layout_mlp(nn, layers, layers+3);
	
	train(nn, 100000);
	write_graphviz(std::cout, nn);	
	
	return 0;
}
