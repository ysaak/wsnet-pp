/**
 *  \file   stats.h
 *  \brief  Statistics library
 *  \author Romain Kuntz
 *  \date   01/2010
 **/

#ifndef _STATS_H
#define _STATS_H

#include <include/models.h>

extern FILE *stats_output_file;

/**  
 * \brief Create statistics structures for the calling module.
 * \param c should be {entity id, node id, -1}.
 * \param cleanup_function the function called to cleanup
 *        statistics' entries (i.e. the 'value' parameter 
 *        of stats_add())
 * \return NULL on error, ID of the stats on success.
 **/
void *stats_create(call_t *c, callback_t cleanup_function);

/**  
 * \brief Process statistics for the calling module.
 * \param key the key that stats_init() initially returned.
 * \param callback_function the function that handles the 
 *        statistics.
 * \return 0 on success, -1 on failure.
 **/
int stats_process(void *key, callback_t callback_function);

/**  
 * \brief Add statistics for a module.
 * \param key the key that stats_init() initially returned.
 * \param value the value to store (can be a pointer to a structure).
 * \return 0 on success, -1 on failure.
 **/
int stats_add(void *key, void *value);

/**
 * \brief Set the statistics handler functions.
 * \param c should be {entity id, -1, -1}.
 * \param model_type the model type, see include/models.h.
 * \param handler_function then handler function for the 
 *        specified model type.
 **/
void set_stats_handler(call_t *c, int model_type,
                       void *handler_function);

/**
 * \brief Initialize the statistics' structure. Called by WSnet's main()
 **/
int stats_init(void);

/**
 * \brief Execute the statistics handlers. Called by WSNet's main()
 **/
void do_stats();

/**
 * \brief Clean up statistics. Called by WSNet's main() 
 **/
void stats_clean(void);

/**
 * \brief Initialize statistics' output file. Called by WSNet's 
 *        parse_simulation()
 **/
int stats_set_output(char *output);

#endif /* _STATS_H */

