/**
 *  \file   stats_handler_sample.h
 *  \brief  Definition of the statistics structures
 *  \author Romain Kuntz
 *  \date   01/2010
 **/

/* Application layer structure */
struct _app_stat {
    uint64_t tx_time;
    uint64_t rx_time;
    uint16_t pkt_seq;
};
