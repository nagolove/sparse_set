#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// SPARSE SET

/*
    de_sparse:

    How the components sparse set works?
    The main idea comes from ENTT C++ library:
    https://github.com/skypjack/entt
    https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#views
    (Credits to skypjack) for the awesome library.

    We have an sparse array that maps entity identifiers to the dense array indices that contains the full entity.


    sparse array:
    sparse => contains the index in the dense array of an entity identifier (without version)
    that means that the index of this array is the entity identifier (without version) and
    the content is the index of the dense array.

    dense array:
    dense => contains all the entities (de_entity).
    the index is just that, has no meaning here, it's referenced in the sparse.
    the content is the de_entity.

    this allows fast iteration on each entity using the dense array or
    lookup for an entity position in the dense using the sparse array.

    ---------- Example:
    Adding:
     de_entity = 3 => (e3)
     de_entity = 1 => (e1)

    In order to check the entities first in the sparse, we have to retrieve the de_entity_id part of the de_entity.
    The de_entity_id part will be used to index the sparse array.
    The full de_entity will be the value in the dense array.


                           0    1     2    3
    sparse idx:         eid0 eid1  eid2  eid3    this is the array index based on de_entity_id (NO VERSION)
    sparse content:   [ null,   1, null,   0 ]   this is the array content. (index in the dense array)

    dense         idx:    0    1
    dense     content: [ e3,  e2]
*/

// XXX: Использовать 32х битные индексы или 64битные?
typedef struct koh_SparseSet {
    /*  sparse entity identifiers indices array.
        - index is the de_entity_id. (without version)
        - value is the index of the dense array
    */
    uint32_t*  sparse;
    size_t      sparse_size, sparse_cap;

    /*  Dense entities array.
        - index is linked with the sparse value.
        - value is the full de_entity
    */
    uint32_t*  dense;
    size_t      dense_size, dense_cap;
    size_t      initial_cap;
} de_sparse;

/*
__attribute__((unused))
static void de_sparce_print(de_sparse *s) {
    assert(s);
    for (int i = 0; i < s->dense_size; i++) {
        de_trace("%u ", s->dense[i]);
    }
    de_trace("\n");
}
*/

static de_sparse* de_sparse_init(de_sparse* s) {
    assert(s);
    printf("de_sparse_init: %p\n", s);
    if (s) {
        *s = (de_sparse){ 0 };
        //s->sparse = 0;
        //s->dense = 0;
    }
    return s;
}

/*
static de_sparse* de_sparse_new() {
    return de_sparse_init(malloc(sizeof(de_sparse)));
}
*/

static void de_sparse_destroy(de_sparse* s) {
    assert(s);
    printf("de_sparse_destroy: %p", s);
    if (!s) return;
    if (s->sparse) {
        free(s->sparse);
        s->sparse = NULL;
    }
    if (s->dense) {
        free(s->dense);
        s->dense = NULL;
    }
}

/*
static void de_sparse_delete(de_sparse* s) {
    de_sparse_destroy(s);
    free(s);
}
*/

static const uint32_t de_null = UINT32_MAX;

static bool de_sparse_contains(de_sparse* s, uint32_t e) {
    assert(s);
    assert(e != de_null);
    printf("de_sparse_contains: de_sparse %p, e %u\n", s, e);
    return (e < s->sparse_size) && (s->sparse[e] != de_null);
}

static size_t de_sparse_index(de_sparse* s, uint32_t e) {
    assert(s);
    assert(de_sparse_contains(s, e));
    printf("de_sparse_index: de_sparse %p, e %u\n", s, e);
    return s->sparse[e];
}

static void de_sparse_emplace(de_sparse* s, uint32_t e) {
    assert(s);
    assert(e != de_null);
    printf("de_sparse_emplace: de_sparse %p, e %u\n", s, e);
#ifdef DE_USE_SPARSE_CAPACITY
    const uint32_t_id eid = de_entity_identifier(e);
    if (eid.id >= s->sparse_size) { // check if we need to realloc

        // первоначальное выделение
        if (s->sparse_size == 0 && !s->sparse) {
            s->sparse_cap = s->initial_cap;
            s->sparse = realloc(s->sparse, s->sparse_cap * sizeof(s->sparse[0]));
        }

        const size_t new_sparse_size = eid.id + 1;

        // расширение выделения
        if (new_sparse_size == s->sparse_cap) {
            s->sparse_cap *= 2;
            s->sparse = realloc(s->sparse, s->sparse_cap * sizeof(s->sparse[0]));
        }

        //s->sparse = realloc(s->sparse, new_sparse_size * sizeof(s->sparse[0]));

        //memset(s->sparse + s->sparse_size, de_null, (new_sparse_size - s->sparse_size) * sizeof(s->sparse[0]));
        uint32_t *start = s->sparse + s->sparse_size;
        int bytes_num = (new_sparse_size - s->sparse_size) * sizeof(s->sparse[0]);
        while (bytes_num) {
            *start++ = de_null;
            bytes_num -= sizeof(uint32_t);
        }
        
        s->sparse_size = new_sparse_size;
    }
    s->sparse[eid.id] = (uint32_t)s->dense_size; // set this eid index to the last dense index (dense_size)
    //trace("s->dense_size: %d\n", s->dense_size);

    // первоначальное выделение
    if (s->dense_size == 0 && !s->dense) {
        // TODO: Вынести dense_cap в s->initial_dense_cap
        s->dense_cap = s->initial_cap;
        s->dense = realloc(s->dense, s->dense_cap * sizeof(s->dense[0]));
    }

    // расширение выделения
    if (s->dense_size == s->dense_cap) {
        s->dense_cap *= 2;
        s->dense = realloc(s->dense, s->dense_cap * sizeof(s->dense[0]));
    }

    s->dense[s->dense_size] = e;
    s->dense_size++;
#else
    assert(s);
    assert(e != de_null);
    if (e >= s->sparse_size) { // check if we need to realloc
        const size_t new_sparse_size = e + 1;
        s->sparse = realloc(s->sparse, new_sparse_size * sizeof(s->sparse[0]));
        //memset(s->sparse + s->sparse_size, de_null, (new_sparse_size - s->sparse_size) * sizeof(s->sparse[0]));
      
        uint32_t *start = s->sparse + s->sparse_size;
        int bytes_num = (new_sparse_size - s->sparse_size) * sizeof(s->sparse[0]);
        while (bytes_num) {
            *start++ = de_null;
            bytes_num -= sizeof(uint32_t);
        }
        
        s->sparse_size = new_sparse_size;
    }
    s->sparse[e] = (uint32_t)s->dense_size; // set this eid index to the last dense index (dense_size)
    s->dense = realloc(s->dense, (s->dense_size + 1) * sizeof(s->dense[0]));
    s->dense[s->dense_size] = e;
    s->dense_size++;
#endif
}

static size_t de_sparse_remove(de_sparse* s, uint32_t e) {
    assert(s);
    assert(de_sparse_contains(s, e));
    printf("de_sparse_remove: de_sparse %p, e %u\n", s, e);
#ifdef DE_USE_SPARSE_CAPACITY

    const size_t pos = s->sparse[uint32_t_identifier(e).id];
    const uint32_t other = s->dense[s->dense_size - 1];

    s->sparse[uint32_t_identifier(other).id] = (de_entity)pos;
    s->dense[pos] = other;
    s->sparse[pos] = de_null;

    /*
    // XXX: Возможно ошибка при работе с памятью
    if (s->dense_size < 0.5 * s->dense_cap) {
        s->dense_cap /= 2;
        s->dense = realloc(s->dense, s->dense_cap * sizeof(s->dense[0]));
    }
    */

    s->dense_size--;

    return pos;
#else

    const size_t pos = s->sparse[e];
    const uint32_t other = s->dense[s->dense_size - 1];

    s->sparse[other] = (uint32_t)pos;
    s->dense[pos] = other;
    s->sparse[pos] = de_null;

    s->dense = realloc(s->dense, (s->dense_size - 1) * sizeof(s->dense[0]));
    s->dense_size--;

    return pos;
#endif
}

