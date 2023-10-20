#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "library_math.h"
#include "library.h"
#include "full_connected.h"

#define activation_f(x) Sigmoid.f(x)
#define activation_df(x) Sigmoid.df(x)
#define loss_f(ideal, output) MSE_f(ideal, output)
#define loss_df(ideal, output) MSE_df(ideal, output)


FullConnected* FullConnected_construct(int layers_length, int* layers, double learning_rate, double momentum) {
	FullConnected* layer = malloc(sizeof(FullConnected));
	layer->layers_length = layers_length;
	layer->layers = layers;
	layer->learning_rate = learning_rate;
	layer->momentum = momentum;

	layer->weights = (double***)malloc(sizeof(double**) * (layers_length - 1));
	for (int i = 0; i < layers_length; ++i) {
		layer->weights[i] = (double**)malloc(sizeof(double*) * layers[i]);
		for (int j = 0; j < layers[i]; ++j) {
			layer->weights[i][j] = (double*)malloc(sizeof(double) * layers[i + 1]);
			for (int l = 0; l < layers[i + 1]; ++l) {
				layer->weights[i][j][l] = random(-1, 1);
			}
		}
	}

	layer->x = (double**)malloc(sizeof(double*) * layers_length);
	for (int i = 0; i < layers_length; ++i) {
		layer->x[i] = (double*)malloc(sizeof(double) * layers[i]);
		for (int j = 0; j < layers[i]; ++j) {
			layer->x[i][j] = 0;
		}
	}

	layer->biases = (double**)malloc(sizeof(double*) * (layers_length - 1));
	for (int i = 0; i < layers_length; ++i) {
		layer->biases[i] = (double*)malloc(sizeof(double) * layers[i + 1]);
		for (int j = 0; j < layers[i + 1]; ++j) {
			layer->biases[i][j] = random(-1, 1);
		}
	}

	return layer;
}

void FullConnected_deconstruct(FullConnected* layer) {
	for (int i = 0; i < layer->layers_length - 1; ++i) {
		for (int j = 0; j < layer->layers[i]; ++j) {
			free(layer->weights[i][j]);
		}
		free(layer->weights[i]);
	}
	free(layer->weights);

	for (int i = 0; i < layer->layers_length; ++i) {
		free(layer->x[i]);
	}
	free(layer->x);

	for (int i = 0; i < layer->layers_length - 1; ++i) {
		free(layer->biases[i]);
	}
	free(layer->biases);
	
	free(layer);
}

void FullConnected_train_construct(FullConnected* layer) {
	layer->gradient = (double**)malloc(sizeof(double*) * layer->layers_length);
	for (int i = 0; i < layer->layers_length; ++i) {
		layer->gradient[i] = (double*)malloc(sizeof(double) * layer->layers[i]);
		for (int j = 0; j < layer->layers[i]; ++j) {
			layer->gradient[i][j] = 0;
		}
	}

	layer->delta_weights = (double***)malloc(sizeof(double**) * (layer->layers_length - 1));
	for (int i = 0; i < layer->layers_length; ++i) {
		layer->delta_weights[i] = (double**)malloc(sizeof(double*) * layer->layers[i]);
		for (int j = 0; j < layer->layers[i]; ++j) {
			layer->delta_weights[i][j] = (double*)malloc(sizeof(double) * layer->layers[i + 1]);
			for (int l = 0; l < layer->layers[i + 1]; ++l) {
				layer->delta_weights[i][j][l] = 0;
			}
		}
	}

	layer->delta_biases = (double**)malloc(sizeof(double*) * (layer->layers_length - 1));
	for (int i = 0; i < layer->layers_length; ++i) {
		layer->delta_biases[i] = (double*)malloc(sizeof(double) * layer->layers[i + 1]);
		for (int j = 0; j < layer->layers[i + 1]; ++j) {
			layer->delta_biases[i][j] = 0;
		}
	}
}

void FullConnected_train_deconstruct(FullConnected* layer) {
	for (int i = 0; i < layer->layers_length; ++i) {
		free(layer->gradient[i]);
	}
	free(layer->gradient);

	for (int i = 0; i < layer->layers_length - 1; ++i) {
		for (int j = 0; j < layer->layers[i]; ++j) {
			free(layer->delta_weights[i][j]);
		}
		free(layer->delta_weights[i]);
	}
	free(layer->delta_weights);

	for (int i = 0; i < layer->layers_length - 1; ++i) {
		free(layer->delta_biases[i]);
	}
	free(layer->delta_biases);
}

void FullConnected_forward(FullConnected* layer, double* input) {
	for (int i = 0; i < layer->layers[0]; ++i) {
		layer->x[0][i] = input[i];
	}

	for (int i = 0; i < layer->layers_length - 1; ++i) {
		for (int l = 0; l < layer->layers[i + 1]; ++l) {
			layer->x[i + 1][l] = layer->biases[i][l];
			for (int j = 0; j < layer->layers[i]; ++j) {
				layer->x[i + 1][l] += layer->weights[i][j][l] * layer->x[i][j];
			}
			if (i + 1 < layer->layers_length - 1) {
				layer->x[i + 1][l] = activation_f(layer->x[i + 1][l]);
			}
		}
	}
	SoftMax_f(layer->layers[layer->layers_length - 1], layer->x[layer->layers_length - 1], layer->x[layer->layers_length - 1]);
}

void FullConnected_backward(FullConnected* layer, double* output) {
	for (int i = 0; i < layer->layers[layer->layers_length - 1]; ++i) {
		layer->gradient[layer->layers_length - 1][i] = loss_df(output[i], layer->x[layer->layers_length - 1][i]) * SoftMax_df(layer->x[layer->layers_length - 1][i]);
	}

	for (int i = layer->layers_length - 2; i >= 0; --i) {
		for (int j = 0; j < layer->layers[i]; ++j) {
			layer->gradient[i][j] = 0;
			for (int l = 0; l < layer->layers[i + 1]; ++l) {
				layer->gradient[i][j] += layer->gradient[i + 1][l] * layer->weights[i][j][l] * activation_df(layer->x[i][j]);
			}
		}
	}

	for (int i = 0; i < layer->layers_length - 1; ++i) {
		for (int l = 0; l < layer->layers[i + 1]; ++l) {
			for (int j = 0; j < layer->layers[i]; ++j) {
                layer->delta_weights[i][j][l] = -layer->learning_rate * layer->x[i][j] * layer->gradient[i + 1][l] + layer->momentum * layer->delta_weights[i][j][l];
                layer->weights[i][j][l] += layer->delta_weights[i][j][l];
			}
            
            layer->delta_biases[i][l] = -layer->learning_rate * layer->gradient[i + 1][l] + layer->momentum * layer->delta_biases[i][l];
            layer->biases[i][l] += layer->delta_biases[i][l];
		}
	}
}

double FullConnected_calculate_error(FullConnected* layer, DataSet* data_set) {
	double error = 0;
	for (int i = 0; i < data_set->length; ++i) {
		FullConnected_forward(layer, data_set->pairs[i]->input);
		for (int j = 0; j < data_set->output_length; ++j) {
			error += fabs(data_set->pairs[i]->output[j] - layer->x[layer->layers_length - 1][j]);
		}
		error /= data_set->output_length;
	}
	error /= data_set->length;

	return error;
}

void FullConnected_train(FullConnected* layer, DataSet* data_set, int epochs) {
	int index;
	for (int i = 0; i < epochs; ++i) {
		index = random_int(0, data_set->length - 1);

		FullConnected_forward(layer, data_set->pairs[index]->input);
		FullConnected_backward(layer, data_set->pairs[index]->output);
	}

	printf("error %lf\n", FullConnected_calculate_error(layer, data_set));
}

int FullConnected_train_alpha(FullConnected* layer, DataSet* data_set, double alpha) {
	int step = 200;

	double error = FullConnected_calculate_error(layer, data_set);
	int epochs = 0;
	while (error > alpha) {
		FullConnected_train(layer, data_set, step);

		epochs += step;

		error = FullConnected_calculate_error(layer, data_set);
		printf("error %lf\n", error);
	}

	return epochs;
}

void FullConnected_check(FullConnected* layer, DataSet* data_set) {
	for (int i = 0; i < data_set->length; ++i) {
		FullConnected_forward(layer, data_set->pairs[i]->input);
		printf("%d {", i);
		for (int j = 0; j < data_set->output_length; ++j) {
			if (j < data_set->output_length - 1) {
				printf("%lf, ", layer->x[layer->layers_length - 1][j]);
			} else {
				printf("%lf", layer->x[layer->layers_length - 1][j]);
			}
		}
		printf("} -> ");

		printf("{");
		for (int j = 0; j < data_set->output_length; ++j) {
			if (j < data_set->output_length - 1) {
				printf("%lf, ", data_set->pairs[i]->output[j]);
			} else {
				printf("%lf", data_set->pairs[i]->output[j]);
			}
		}
		printf("}\n");
	}
}
