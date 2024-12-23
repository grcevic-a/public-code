import argparse
import random
import re
import numpy as np


class GeneticAlgorithm:
    def __init__(self, input_data, y, layers_list, popsize, elitism, p, K, iterations):
        self.input_data = input_data
        self.y = y
        self.layers_list = layers_list

        self.popsize = int(popsize)
        self.elitism = int(elitism)
        self.p = float(p)
        self.K = float(K)
        self.iterations = int(iterations)

    def population_initialization(self):
        population = []
        for _ in range(0, self.popsize):
            temp = NeuralNetwork(self.input_data, self.y, self.layers_list)
            temp.initialization()
            temp.calculate(self.input_data)
            population.append([1/temp.mean_sqrd_err(), temp])  # sortiranje prema dobroti
        return sorted(population, key=lambda x: x[0], reverse=True)

    def roulette_wheel(self, population):
        networks = [x[1] for x in population]
        fitness_sum = sum(fitness[0] for fitness in population)
        p_copy = population.copy()
        prob = [float(individual[0] / fitness_sum) for individual in p_copy]
        parents = np.random.choice(networks, 2, p=prob, replace=False)
        return parents

    def train(self, population):
        for i in range(self.iterations):
            population_new = []
            # elitizam
            for _ in range(self.elitism):
                population_new.append(population.pop(0))

            while len(population_new) < self.popsize:
                # selekcija
                parents = self.roulette_wheel(population)

                # krizanje
                offspring = NeuralNetwork(self.input_data, self.y, self.layers_list)
                offspring.initialization()
                for j in range(len(parents[0].w_n)):
                    offspring.w_n[j] = ((np.array(parents[0].w_n[j]) + np.array(parents[1].w_n[j])) / 2).tolist()
                    offspring.biases_vector[j] = ((np.array(parents[0].biases_vector[j]) + np.array(parents[1].biases_vector[j])) / 2).tolist()

                # mutacija
                if random.random() < self.p:
                    for j in range(len(offspring.w_n)):
                        offspring.w_n[j] = [(x + y) for x, y in zip(offspring.w_n[j], np.random.normal(scale=self.K, size=(len(offspring.w_n[j]))))]
                        offspring.biases_vector[j] = [(x + y) for x, y in zip(offspring.biases_vector[j], np.random.normal(scale=self.K, size=(len(offspring.w_n[j]))))]

                offspring.calculate(self.input_data)
                population_new.append([1/offspring.mean_sqrd_err(), offspring])
            population = sorted(population_new, key=lambda x: x[0], reverse=True)

            if ((i+1) % 2000) == 0:
                print("[Train error @{:d}]: {:f}".format(i+1, float(1/population[0][0])))

        return population[0][1]

    def test(self, X_test, y_test, best_n):
        best_n.calculate(X_test)
        best_n.y = y_test
        print("[Test error]: {:f}".format(best_n.mean_sqrd_err()))


class NeuralNetwork:
    def __init__(self, input_data, y, layers_list):
        self.y = y
        self.layers_list = layers_list

        self.w_n = []
        self.biases_vector = []
        self.calc_with = input_data
        self.obtained_output = []
        self.error = 0

    def initialization(self):
        for i in range(1, len(self.layers_list)):
            w = np.random.normal(scale=0.01, size=(int(self.layers_list[i-1]), int(self.layers_list[i])))
            b = np.random.normal(scale=0.01, size=(1, int(self.layers_list[i])))
            self.w_n.append(w.tolist())
            self.biases_vector.append(b.tolist())
        #print(self.w_n)

    def calculate(self, X):
        for x in X:
            for i in range((len(self.w_n) - 1)):
                pom = np.dot(x, self.w_n[i]) + self.biases_vector[i]  # matricno mnozenje + pomak
                x = 1 / (1 + np.exp(-pom))  # prijenosna funkcija
            self.obtained_output.append(np.dot(x, self.w_n[-1]) + self.biases_vector[-1])

    def mean_sqrd_err(self):
        mse = 0
        for i in range(len(self.y)):
            mse += np.square(float(self.obtained_output[i]) - float(self.y[i]))
        self.error = mse / len(self.y)
        self.obtained_output = []
        return self.error


# citaju se datoteke za treniranje i testiranje
def read_input(path_t):
    X = []
    y = []
    with open(path_t, "r", encoding='utf-8') as f:
        lines = f.readlines()
        lines.pop(0)
        for line in lines:
            pom = line.strip().split(',')
            float_list_X = [float(element) for element in pom]
            X.append(float_list_X[:-1])
            y.append(float_list_X[-1])
    return X, y


def main():
    parser = argparse.ArgumentParser(description='Load files.')
    parser.add_argument("--train", action="store")    # putanja do skupa za treniranje
    parser.add_argument("--test", action="store")     # putanja do skupa za testiranje
    parser.add_argument("--nn", action="store")       # arhitektura neuronske mreze
    parser.add_argument("--popsize", action="store")  # velicina populacije
    parser.add_argument("--elitism", action="store")  # elitizam
    parser.add_argument("--p", action="store")        # vjerojatnost mutacije
    parser.add_argument("--K", action="store")        # standardna devijacija
    parser.add_argument("--iter", action="store")     # broj iteracija
    args = parser.parse_args()

    X, y = read_input(args.train)
    neurons_in_layers = re.findall(r'\d+', args.nn)
    neurons_in_layers.insert(0, len(X[0]))
    neurons_in_layers.append(1)

    alg = GeneticAlgorithm(X, y, neurons_in_layers, args.popsize, args.elitism, args.p, args.K, args.iter)
    population = alg.population_initialization()
    #print(population)
    best_n = alg.train(population)
    '''
    n = NeuralNetwork(X, y, neurons_in_layers)
    n.initialization()
    n.calculate(X)
    print(n.obtained_output)
    '''
    X_test, y_test = read_input(args.test)
    alg.test(X_test, y_test, best_n)


if __name__ == "__main__":
    main()
