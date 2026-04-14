#include "mnist_nn.h"

#include <math.h>
#include <stdint.h>

// Importation des poids
#include "mnist_weights.h"

// Requantification, avec :
// - x : entier 32 bits representant une activation avant la requantification
// - mul : multiplicateur de requantification pour ajuster les activations apres la multiplication
int32_t requantize(int32_t x, int32_t mul) {
  int64_t tmp = (int64_t)x * (int64_t)mul;
  tmp += 1LL << (MNIST_MUL_SHIFT - 1);
  return (int32_t)(tmp >> MNIST_MUL_SHIFT);
}

// Fonction ReLU, tenant compte de la quantification, avec :
// - input : tableau d'entiers 32 bits representant les activations de la couche precedente
// - output : tableau d'entiers 32 bits pour stocker les activations de sortie de la couche ReLU
// - size : taille du tableau d'entiers 32 bits representant les activations de la couche precedente
// - mul : multiplicateur de requantification pour ajuster les activations apres la multiplication
void relu_layer(const int32_t *input, int32_t *output, int size, int32_t mul) {
  for (int i = 0; i < size; ++i) {
    int32_t x = requantize(input[i], mul);

    if (x < MNIST_ACT_MIN) {
      x = MNIST_ACT_MIN;
    }
    if (x > MNIST_ACT_MAX) {
      x = MNIST_ACT_MAX;
    }

    output[i] = x;
  }
}

// Fonction softmax pour obtenir les probabilites a partir des scores de la couche de sortie pour les 10 classes, avec :
// - scores : tableau de floats representant les scores de la couche de sortie pour les 10 classes
// - probabilities : tableau de floats representant les probabilites pour chaque classe
void softmax10(const float scores[MNIST_CLASSES], float probabilities[MNIST_CLASSES]) {
  float maxv = scores[0];

  for (int i = 1; i < MNIST_CLASSES; ++i) {
    if (scores[i] > maxv) {
      maxv = scores[i];
    }
  }

  float s = 0.0f;
  for (int i = 0; i < MNIST_CLASSES; ++i) {
    probabilities[i] = expf(scores[i] - maxv);
    s += probabilities[i];
  }

  for (int i = 0; i < MNIST_CLASSES; ++i) {
    probabilities[i] /= s;
  }
}

// Fonction pour les couches denses, avec :
// - input : tableau d'entiers 32 bits representant les activations de la couche precedente
// - weights : tableau d'entiers 8 bits representant les poids de la couche dense
// - bias_or_base : tableau d'entiers 32 bits representant soit les biais (pour la premiere couche), soit les bases de quantification (pour les couches suivantes)
// - input_size : nombre d'entrees de la couche dense
// - output_size : nombre de sorties de la couche dense
// - output : tableau d'entiers 32 bits pour stocker les activations de sortie de la couche dense
void dense_layer(const int32_t *input, const int8_t *weights,
                 const int32_t *bias_or_base, int input_size,
                 int output_size, int32_t *output) {
  for (int j = 0; j < output_size; ++j) {
    output[j] = bias_or_base[j];
  }

  for (int i = 0; i < input_size; ++i) {
    if (input[i] == 0) {
      continue;
    }

    const int8_t *row = &weights[i * output_size];
    for (int j = 0; j < output_size; ++j) {
      output[j] += input[i] * (int32_t)row[j];
    }
  }
}

// Fonction pour obtenir les probabilites a partir de l'image, avec :
// - image : tableau d'entiers 8 bits representant l'image en niveaux de gris
// - probabilities : tableau de floats representant les probabilites pour chaque classe
void mnist_predict_proba(const uint8_t image[MNIST_PIXELS], float probabilities[MNIST_CLASSES]) {
  int32_t first_layer_input[MNIST_PIXELS];
  int32_t hidden1_before_relu[MNIST_HIDDEN];
  int32_t hidden1[MNIST_HIDDEN];
  int32_t hidden2_before_relu[MNIST_HIDDEN];
  int32_t hidden2[MNIST_HIDDEN];
  int32_t output_scores_q[MNIST_CLASSES];
  float output_scores[MNIST_CLASSES];

  for (int i = 0; i < MNIST_PIXELS; ++i) {
    if (image[i] == 0u) {
      first_layer_input[i] = 0;
    } else {
      first_layer_input[i] = MNIST_LAYER_1_INPUT_DELTA_Q;
    }
  }

  dense_layer(first_layer_input, mnist_layer_1_weights_q, mnist_layer_1_base_q,
              MNIST_PIXELS, MNIST_HIDDEN, hidden1_before_relu);
  relu_layer(hidden1_before_relu, hidden1, MNIST_HIDDEN, mnist_layer_1_requant_mul);

  dense_layer(hidden1, mnist_layer_2_weights_q, mnist_layer_2_bias_q,
              MNIST_HIDDEN, MNIST_HIDDEN, hidden2_before_relu);
  relu_layer(hidden2_before_relu, hidden2, MNIST_HIDDEN, mnist_layer_2_requant_mul);

  dense_layer(hidden2, mnist_layer_3_weights_q, mnist_layer_3_bias_q,
              MNIST_HIDDEN, MNIST_CLASSES, output_scores_q);

  for (int c = 0; c < MNIST_CLASSES; ++c) {
    output_scores[c] = (float)output_scores_q[c] * mnist_layer_3_output_scale;
  }

  softmax10(output_scores, probabilities);
}
