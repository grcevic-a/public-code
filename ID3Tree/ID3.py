import sys
import math


class Node:
    def __init__(self, data, subtrees):
        self.data = data
        self.next = subtrees

    def __repr__(self):
        return self.data + ": " + str(self.next)

    def __str__(self):
        return self.data + ": " + str(self.next)


class Leaf:
    def __init__(self, data):
        self.data = data

    def __repr__(self):
        return self.data

    def __str__(self):
        return self.data


class ID3:
    def __init__(self, dict_of_set):
        self.dict_of_set = dict_of_set
        self.depth = None

    # metoda koja pronalazi najčešću oznaku skupa podataka i vraća filtrirani skup i najčešću oznaku
    def argmax(self, d, y):
        dict_y_n = {}
        y = sorted(y)
        for i in y:
            dict_y_n[i] = []

        for val in d:
            for key in dict_y_n:
                if key == val[-1]:
                    dict_y_n[key].append(val)

        longest = 0
        y_or_no = ""
        new_d = []
        for key in dict_y_n:
            if longest < len(dict_y_n[key]):
                longest = len(dict_y_n[key])
                new_d = dict_y_n[key]
                y_or_no = key
        return new_d, y_or_no

    # metoda koja pronalazi najdiskriminativniju značajku
    def IG(self, d, x, e_value, y):
        ig = {}
        for key in x:
            information = 0
            for val in self.dict_of_set[key]:
                pos = x.index(key)
                e = entropy(d, val, y, pos)
                counter = 0
                for atr in d:
                    if val == atr[pos]:
                        counter += 1
                information += (counter / len(d)) * e
            ig[key] = round(e_value - information, 4)
        ig = dict(sorted(ig.items(), key=lambda x: (-x[1], x[0]))) #https://www.askpython.com/python/dictionary/sort-a-dictionary-by-value-in-python
        return next(iter(ig.items()))

    # metoda u kojoj model uči nad podacima za učenje
    # metoda može učiti na čitavom skupu za treniranje, ali se može i ograničiti dubina kako bi se izbjegla prenučenost
    def fit(self, d, parent_d, x, y, e_value, count):
        if len(d) == 0:
            v_d, y_n = self.argmax(parent_d, y)
            return Leaf(y_n)
        if count == 0:
            v_d, y_n = self.argmax(d, y)
            return Leaf(y_n)
        v_d, y_n = self.argmax(d, y)
        if len(x) == 0 or d == v_d:
            return Leaf(y_n)

        ig = self.IG(d, x, e_value, y)
        el_x = ig[0]
        subtrees = {}
        for val in self.dict_of_set[el_x]:
            new_d = []
            new_x = []
            counter = 0
            for elements in x:
                if elements != el_x:
                    new_x.append(elements)
                elif elements == el_x:
                    counter = x.index(elements)
            for i in d:
                pom = i.copy()
                if val == i[counter]:
                    pom.pop(counter)
                    new_d.append(pom)
            if count is not None:
                count_new = count
                count_new -= 1
            else:
                count_new = None
            t = self.fit(new_d, d, new_x, y, entropy(new_d, None, y, None), count_new)
            subtrees[val] = t
        return Node(el_x, subtrees)

    # metoda u kojoj model generira predikcije na skupu za testiranje
    def predict(self, tree, testing, x, root):
        p = ""
        pom_dict = {}
        for head in x:
            if head == root:
                position = x.index(head)
                for key in tree:
                    if testing[position] == key:
                        if type(tree[key]) is Leaf:
                            return tree[key]
                        p = self.predict(tree[key].next, testing, x, tree[key].data)
                        break
                if testing[position] not in tree.keys():
                    for k in tree:
                        if tree[k].data not in pom_dict.keys():
                            pom_dict[tree[k].data] = 1
                        else:
                            pom_dict[tree[k].data] += 1
                    pom_dict = dict(sorted(pom_dict.items(), key=lambda x: (-x[1], x[0])))
                    p = next(iter(pom_dict.keys()))
        return p

    # metoda za ispis stabla
    def print_tree(self, tree, branches, root):
        if type(tree) is Leaf:
            counter = 1
            for branch in branches:
                print(str(counter) + ":" + str(branch[0]) + "=" + str(branch[1]), end=" ")
                counter += 1
            print(tree)
        else:
            iter_dict = tree.next
            for key in iter_dict:
                branches.append([root, key])
                self.print_tree(iter_dict[key], branches, iter_dict[key].data)
                branches.pop()

    def accuracy(self, testing, predictions):
        counter = 0
        correct = 0
        for t in testing:
            if str(t[-1]) == str(predictions[counter]):
                correct += 1
            counter += 1
        return round(correct/len(testing), 5)

    def confusion_matrix(self, testing, predictions, y):
        matrix = []
        pom = {}
        y = sorted(y)
        for real in y:
            for y_n in y:
                pom[y_n] = 0
            for predict in y:
                counter = 0
                for t in testing:
                    if str(t[-1]) == str(real) and str(predictions[counter]) == str(predict):
                        pom[predict] += 1
                    counter += 1
            for y_n in y:
                matrix.append(pom[y_n])
        return matrix


def entropy(set_t, val, y_n, key):
    dict_y_n = {}
    for i in y_n:
        dict_y_n[i] = 0

    for atributes in set_t:
        if val is None:
            for i in y_n:
                if atributes[-1] == i:
                    dict_y_n[i] += 1
        else:
            if val == atributes[key]:
                for i in y_n:
                    if atributes[-1] == i:
                        dict_y_n[i] += 1
    suma = 0

    for key in dict_y_n:
        suma += dict_y_n[key]

    entropy_val = 0
    for key in dict_y_n:
        if dict_y_n[key] == 0:
            entropy_val -= 0
        else:
            entropy_val -= (dict_y_n[key] / suma) * math.log2(dict_y_n[key] / suma)
    return entropy_val


def get_all_values(header, set_t):
    dict_of_set = {}
    for i in range(0, len(header)):
        for atribute in set_t:
            if header[i] in dict_of_set and atribute[i] not in dict_of_set[header[i]]:
                dict_of_set[header[i]].append(atribute[i])
            elif header[i] not in dict_of_set:
                dict_of_set[header[i]] = [atribute[i]]
    return dict_of_set


def read_input(pathCSV):
    count = 0
    training = []
    header = []

    f = open(pathCSV, "r", encoding='utf-8')
    lines = f.readlines()

    for line in lines:
        if count == 0:
            header = line.strip().split(',')
            count = 1
        else:
            training.append(line.strip().split(','))
    return header, training


def main():
    header, training = read_input(sys.argv[1])
    dict_of_set = get_all_values(header, training)
    e_value = entropy(training, None, dict_of_set[header[-1]], None)
    model = ID3(dict_of_set)

    if len(sys.argv) > 3:
        model.depth = sys.argv[3]

    y = header.pop()

    if model.depth is not None:
        count = int(model.depth)
    else:
        count = None

    tree = model.fit(training, training, header, dict_of_set[y], e_value, count)
    #print(tree)

    print("[BRANCHES]:")
    model.print_tree(tree, [], tree.data)

    header_testing, testing = read_input(sys.argv[2])
    predictions = []
    for t in testing:
        predictions.append(model.predict(tree.next, t, header, tree.data))
    for_print = ""
    print("[PREDICTIONS]:", end=" ")
    for p in predictions:
        for_print += str(p)
        for_print += " "
    print(for_print[:len(for_print) - 1])

    accuracy = model.accuracy(testing, predictions)
    print("[ACCURACY]: ", end="")
    print(f'{accuracy:.5f}')

    confusion = model.confusion_matrix(testing, predictions, dict_of_set[y])
    print("[CONFUSION_MATRIX]:")
    rows = len(dict_of_set[y])
    for i in range(0, len(confusion)):
        if i % rows == 0 and i != 0:
            print()
        print(confusion[i], end=" ")
    print()


if __name__ == "__main__":
    main()
