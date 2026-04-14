#pragma once

#include <stdint.h>

// Parametres MNIST
#define MNIST_W 28
#define MNIST_H 28
#define MNIST_PIXELS 784
#define MNIST_CLASSES 10

// Parametres de quantification
#define MNIST_ACT_MIN 0
#define MNIST_ACT_MAX 127

int32_t requantize(int32_t x, int32_t mul);

void relu_layer(const int32_t *input, int32_t *output, int size, int32_t mul);

void softmax10(const float scores[MNIST_CLASSES], float probabilities[MNIST_CLASSES]);

void dense_layer(const int32_t *input, const int8_t *weights,
                 const int32_t *bias_or_base, int input_size,
                 int output_size, int32_t *output);

void mnist_predict_proba(const uint8_t x01[MNIST_PIXELS], float out_proba[MNIST_CLASSES]);
