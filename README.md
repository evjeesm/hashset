# Hash Set
![check](https://github.com/evjeesm/hashset/actions/workflows/makefile.yml/badge.svg)

My recreation of well known fast lookup data structure.
Includes common set operations.

This project is a part of [vector](https://github.com/evjeesm/vector) hierarchy.

## Implementation details

Collision resolution performed using open addressing with linear probing.

Hashset will grow x2 when reaches maximum capacity and will consequently perform rehashing of all elements.


