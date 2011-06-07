/**
 *  \file   stats_handler_sample.c
 *  \brief  Sample statistic handler module
 *  \author Romain Kuntz
 *  \date   01/2010
 **/
#include <stdio.h>
#include <include/modelutils.h>
#include "stats_handler_sample.h"

/* ************************************************** */
/* ************************************************** */
model_t model =  {
    "Statistic handler module",
    "Romain Kuntz",
    "0.1",
    MODELTYPE_ENVIRONMENT, 
    {NULL, 0}
};

/* ************************************************** */
/* ************************************************** */
/* Handler function for all application statistics */
void app_handler(call_t *c, void *das_all) {
    void *das_node = NULL;
    struct _app_stat *stat = NULL;
    int total_rx = 0, total_tx = 0;

    PRINT_STATISTICS("\n=== Application statistics handler ===\n");

    das_init_traverse(das_all);
    while ((das_node = das_traverse(das_all)) != NULL) {
        das_init_traverse(das_node);

        while ((stat = (struct _app_stat *) 
                    das_traverse(das_node)) != NULL) {
            if (stat->rx_time) {
                total_rx++;
            } else {
                total_tx++;
            }
        }
    }

    PRINT_STATISTICS("%d packets sent and %d received.\n", 
            total_tx, total_rx);
    PRINT_STATISTICS("Packet received: %.2f percent.\n", 
            ((float) total_rx*100)/total_tx);

    return;
}

/* ************************************************** */
/* ************************************************** */
int init(call_t *c, void *params) {
    return 0;
}

int destroy(call_t *c) {
    // cleanup the structure with the stat handlers
    return 0;
}

/* ************************************************** */
/* ************************************************** */
int bootstrap(call_t *c) {
    /* Set the statistic handler for applications */
    set_stats_handler(c, MODELTYPE_APPLICATION, app_handler);

    return 0;
}

int ioctl(call_t *c, int option, void *in, void **out) {
    return 0;
}

/* ************************************************** */
/* ************************************************** */
void read_measure(call_t *c, measureid_t measure, double *value) {
    /* The read_measure() function is not used in this model */
    return;
}
/* ************************************************** */
environment_methods_t methods = {read_measure};

