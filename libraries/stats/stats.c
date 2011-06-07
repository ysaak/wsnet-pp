/**
 *  \file   stats.c
 *  \brief  Statistics library
 *  \author Romain Kuntz
 *  \date   01/2010
 **/
#include <stdio.h>
#include <stdarg.h>
#include <include/modelutils.h>
#include <include/stats.h>

/**
 * \typedef stats_t
 * \brief A structure that identifies statistics for an entity.
 **/
/** \struct _stats
 * \brief A structure that identifies statistics for an entity. Should use type stats_t.
 **/
typedef struct _stats {
    call_t *c;
    void *das;
    callback_t cleanup_function;
} stats_t;

typedef struct _stats_handler {
    call_t *c;
    int model_type;
    void (*handler_function)(call_t *c, void *das);
} stats_handler_t;

static void *models_handler = NULL;
static void *statistics = NULL;
static long hash_index = 1;
FILE *stats_output_file = NULL;

// Defined in src/internals.h of WSNet
uint64_t scheduler_get_end(void);

// Functions used for hadas
unsigned long stats_hash(void *key) {
    return (unsigned long) key;
}
int stats_equal(void *key0, void *key1) {
    return (int) (key0 == key1);
}

/**
 * \brief Create statistics structures for the calling module.
 * \param c should be {entity id, node id, -1}.
 * \param cleanup_function the function called to cleanup
 *        statistics' entries (i.e. the 'value' parameter
 *        of stats_add())
 * \return NULL on error, ID of the stats on success.
 **/
void *stats_create(call_t *c, callback_t cleanup_function) {
    stats_t *stats = NULL;

    if (cleanup_function == NULL) {
        fprintf(stderr, "stats_create: cleanup function must be set\n");
        return NULL;
    }

    if (c == NULL) {
        fprintf(stderr, "stats_create: caller (call_t *c) must be set\n");
        return NULL;
    }

    stats = malloc(sizeof(stats_t));
    if (stats == NULL ) {
        fprintf(stderr, "stats_create: malloc failed\n");
        return NULL;
    }

    stats->c = malloc(sizeof (call_t));
    if (stats->c == NULL) {
        fprintf(stderr, "stats_create: malloc failed\n");
        free(stats);
        return NULL;
    }

    /* Initialize the stats structure */
    memcpy(stats->c, c, sizeof (call_t));
    stats->das = das_create();
    stats->cleanup_function = cleanup_function;

    /* Statistic structure is stored in a hadas */
    hadas_insert(statistics, (void *) hash_index, (void *) stats);
    void *key = (void *) hash_index;
    hash_index++;

    return key;
}

/**
 * \brief Process statistics for the calling module.
 * \param key the key that stats_init() initially returned.
 * \param callback_function the function that handles the
 *        statistics.
 * \return 0 on success, -1 on failure.
 **/
int stats_process(void *key, callback_t callback_function) {
    stats_t *stats = (stats_t *) hadas_get(statistics, key);

    if (stats == NULL || callback_function == NULL) {
        fprintf(stderr, "stats_process: error\n");
        return -1;
    }

    callback_function(stats->c, stats->das);
    return 0;
}

/**
 * \brief Add statistics .
 * \param key the key that stats_init() initially returned.
 * \param value the value to store (can be a pointer to a structure).
 * \return 0 on success, -1 on failure.
 **/
int stats_add(void *key, void *value) {
    stats_t *stats = (stats_t *) hadas_get(statistics, key);

    if (stats == NULL) {
        fprintf(stderr, "stats_add: no stats found for this key\n");
        return -1;
    }

    das_insert(stats->das, value);

    return 0;
}

/**
 * \brief Set the statistics handler functions.
 * \param c should be {entity id, -1, -1}.
 * \param model_type themodel type, see include/models.h.
 * \param handler_function then handler function for the
 *        specified model type.
 **/
void set_stats_handler(call_t *c, int model_type,
                       void *handler_function) {
    stats_handler_t *handler = malloc(sizeof(stats_handler_t));

    handler->model_type = model_type;
    handler->handler_function = handler_function;
    handler->c = c;

    /* Add the handler function in the das */
    das_insert(models_handler, (void *) handler);

    /* Handler functions will be called at the end of
     * the simulation through the do_stats() function */
    return;
}

/**
 * \brief Internal function that returns statistics for a certain
 *        model and node
 **/
void *get_model_stats(int model_type, int node_id) {
    long i = 0;
    stats_t *stats = NULL;

    for (i = 0; i < hash_index; i++) {
        void *key = (void *) i;

        if ((stats = (stats_t *) hadas_get(statistics, key)) != NULL) {
            if (get_entity_type(stats->c) == model_type
                    && stats->c->node == node_id) {
                return stats->das;
            }
        }
    }

    return NULL;
}

/**
 * \brief Initialize the statistics' structure. Called by WSNet's main()
 **/
int stats_init(void) {
    statistics = hadas_create(stats_hash, stats_equal);
    models_handler = das_create();
    return 0;
}

/**
 * \brief Execute the statistics handlers. Called by WSNet's main()
 **/
void do_stats() {
    stats_handler_t *handler = NULL;

    das_init_traverse(models_handler);
    while ((handler = (stats_handler_t *) das_traverse(models_handler)) != NULL) {
        void *das_node = NULL;
        void *das_all = das_create();
        nodeid_t node = 0;

        /* Get stats of the same model type for all the nodes */
        for(node = 0; node < get_node_count(); node++) {
            das_node = get_model_stats(handler->model_type, node);

            /* Insert the stats in a das */
            if (das_node != NULL) {
                das_insert(das_all, (void *) das_node);
            }
        }

        /* Call the appropriate handler */
        handler->handler_function(handler->c, das_all);

        /* Destroy the das that gathered all the stats */
        das_destroy(das_all);
    }
}

/**
 * \brief Clean up statistics. Called by WSNet's main()
 **/
void stats_clean(void) {
    stats_handler_t *handler = NULL;
    stats_t *stats = NULL;
    void *das_entry = NULL;
    long i = 0;

    for (i = 0; i < hash_index; i++) {
        void *key = (void *) i;

        if ((stats = (stats_t *) hadas_get(statistics, key)) != NULL) {
            /* Cleanup the das */
            while ((das_entry = (void *) das_pop(stats->das)) != NULL) {
                stats->cleanup_function(stats->c, das_entry);
            }
            das_destroy(stats->das);

            /* Cleanup the stats_t structure */
            free(stats->c);
            free(stats);

            /* Remove the stats_t structure from the hadas */
            hadas_delete(statistics, key);
        }
    }

    if (stats_output_file != NULL) {
        fclose(stats_output_file);
    }

    while ((handler = (stats_handler_t *)
                        das_pop(models_handler)) != NULL) {
        free(handler);
    }

    das_destroy(models_handler);
    hadas_destroy(statistics);
}

/**
 * \brief Initialize statistics' output file. Called by WSNet's
 *        parse_simulation()
 **/
int stats_set_output(char *output) {
    /* Open the statistics output file */
    if ((stats_output_file = fopen(output, "w+")) == NULL) {
        fprintf(stderr, "stats_init: cannot open file %s, "
                "will write on stderr.\n", output);
        return -1;
    }

    return 0;
}
