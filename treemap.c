#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treemap.h"

typedef struct TreeNode TreeNode;


struct TreeNode {
    Pair* pair;
    TreeNode * left;
    TreeNode * right;
    TreeNode * parent;
};

struct TreeMap {
    TreeNode * root;
    TreeNode * current;
    int (*lower_than) (void* key1, void* key2);
};
/*verificar si son iguales*/
int is_equal(TreeMap* tree, void* key1, void* key2){
    if(tree->lower_than(key1,key2)==0 &&  
        tree->lower_than(key2,key1)==0) return 1;
    else return 0;
}

/*Reserva memoria, guarda la clave y el valor, y pone sus punteros left, right y parent en NULL.*/
TreeNode * createTreeNode(void* key, void * value) {
    TreeNode * new = (TreeNode *)malloc(sizeof(TreeNode));
    if (new == NULL) return NULL;
    new->pair = (Pair *)malloc(sizeof(Pair));
    new->pair->key = key;
    new->pair->value = value;
    new->parent = new->left = new->right = NULL;
    return new;
}
/*reserva memoria y define variables*/
TreeMap * createTreeMap(int (*lower_than) (void* key1, void* key2)) {
    TreeMap * map = (TreeMap *)malloc(sizeof(TreeMap));
        if (map == NULL) return NULL;
    
        map->root = NULL;
        map->current = NULL;
        map->lower_than = lower_than;
    
        return map;
}

/*-Si el árbol está vacío, el nuevo nodo se vuelve la raíz.
-Si encuentra una clave igual, no inserta nada (no se permiten duplicados).
-Si la clave es menor, va al subárbol izquierdo.
-Si es mayor, al derecho.
-Crea el nuevo nodo e inserta en la posición encontrada.
Actualiza tree->current al nodo insertado.*/
void insertTreeMap(TreeMap* tree, void* key, void* value) {
    if (tree->root == NULL) {
        tree->root = createTreeNode(key, value);
        tree->current = tree->root;
        return;
    }

    TreeNode* parent = NULL;
    TreeNode* current = tree->root;

    while (current != NULL) {
        parent = current;
        if (is_equal(tree, key, current->pair->key))
            return; // key already exists
        if (tree->lower_than(key, current->pair->key))
            current = current->left;
        else
            current = current->right;
    }

    TreeNode* newNode = createTreeNode(key, value);
    newNode->parent = parent;

    if (tree->lower_than(key, parent->pair->key))
        parent->left = newNode;
    else
        parent->right = newNode;

    tree->current = newNode;
}

/*Devuelve el nodo con la clave mínima en el subárbol con raíz x. 
Para eso baja por la izquierda hasta llegar al último nodo sin hijo izquierdo.*/
TreeNode * minimum(TreeNode * x) {
    if (x == NULL) return NULL;

    while (x->left != NULL) {
        x = x->left;
    }

    return x;
}


/*Elimina un nodo del árbol. Tiene 3 casos:

Sin hijos: Elimina el nodo y desvincula al padre.

Con un hijo: El hijo reemplaza al nodo eliminado.

Con dos hijos: Encuentra el nodo más pequeño del subárbol derecho,
 copia su (clave, valor) en el nodo a eliminar, y luego elimina el nodo mínimo (que tendrá 0 o 1 hijo).*/
void removeNode(TreeMap * tree, TreeNode* node) {
    if (node == NULL) return;

    // Case 1: no children
    if (node->left == NULL && node->right == NULL) {
        if (node->parent == NULL) {
            tree->root = NULL;
        } else {
            if (node->parent->left == node) node->parent->left = NULL;
            else node->parent->right = NULL;
        }
        free(node->pair);
        free(node);
        return;
    }

    // Case 2: one child
    if (node->left == NULL || node->right == NULL) {
        TreeNode* child = node->left ? node->left : node->right;

        if (node->parent == NULL) {
            tree->root = child;
        } else {
            if (node->parent->left == node) node->parent->left = child;
            else node->parent->right = child;
        }

        child->parent = node->parent;
        free(node->pair);
        free(node);
        return;
    }

    // Case 3: two children
    TreeNode* minNode = minimum(node->right);
    node->pair->key = minNode->pair->key;
    node->pair->value = minNode->pair->value;
    removeNode(tree, minNode);
}

/*Busca la clave en el árbol. Si existe, usa removeNode para eliminar el nodo correspondiente.*/
void eraseTreeMap(TreeMap * tree, void* key){
    if (tree == NULL || tree->root == NULL) return;

    if (searchTreeMap(tree, key) == NULL) return;
    TreeNode* node = tree->current;
    removeNode(tree, node);

}

/*Si la encuentra, actualiza tree->current y retorna el par (clave, valor).

Si no, retorna NULL.*/
Pair* searchTreeMap(TreeMap* tree, void* key) {
    TreeNode* node = tree->root;

    while (node != NULL) {
        if (is_equal(tree, key, node->pair->key)) {
            tree->current = node;
            return node->pair;
        }
        if (tree->lower_than(key, node->pair->key))
            node = node->left;
        else
            node = node->right;
    }

    return NULL;
}


/*Devuelve el primer par cuya clave es mayor o igual a key. Si key existe, la devuelve. Si no:

Guarda el candidato ub_node cada vez que encuentra una clave mayor.

Retorna el ub_node con la clave más cercana superior.*/
Pair* upperBound(TreeMap* tree, void* key){
    TreeNode* node = tree->root;
    TreeNode* ub_node = NULL;

    while (node != NULL) {
        if (is_equal(tree, key, node->pair->key)) {
            tree->current = node;
            return node->pair;
        }

        if (tree->lower_than(key, node->pair->key)) {
            ub_node = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }

    if (ub_node != NULL) {
        tree->current = ub_node;
        return ub_node->pair;
    }

    return NULL;
}

/*Retorna el par con la clave más pequeña del árbol. Actualiza el puntero current.*/
Pair * firstTreeMap(TreeMap * tree) {
    if (tree == NULL || tree->root == NULL) return NULL;

    tree->current = minimum(tree->root);
    if (tree->current != NULL)
        return tree->current->pair;
    
    return NULL;
}
/*Retorna el siguiente par en orden creciente de claves:

Si el nodo actual tiene subárbol derecho, retorna el mínimo de ese subárbol.

Si no, sube por los ancestros hasta encontrar un nodo del cual current sea hijo izquierdo.*/
Pair * nextTreeMap(TreeMap * tree) {
    if (tree->current == NULL) return NULL;

    TreeNode * node = tree->current;

    if (node->right != NULL) {
        tree->current = minimum(node->right);
        return tree->current->pair;
    }

    TreeNode* parent = node->parent;
    while (parent != NULL && parent->right == node) {
        node = parent;
        parent = parent->parent;
    }

    tree->current = parent;
    if (tree->current != NULL)
        return tree->current->pair;

    return NULL;
}
