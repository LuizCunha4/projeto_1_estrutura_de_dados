#ifndef ITEM_H
#define ITEM_H

#include <stdio.h>

typedef struct item ITEM;

ITEM *item_criar(const char *asteroid_designation,
                 const char *asteroid_fullname,
                 const char *close_approach_date,
                 int year,
                 int month,
                 double distance_au,
                 double velocity_km_s,
                 double absolute_magnitude,
                 int days_until_approach,
                 int is_past_event,
                 int is_future_event,
                 double risk_score,
                 int panic_level,
                 const char *threat_category,
                 const char *panic_verdict,
                 int on_sentry_list,
                 double sentry_impact_prob,
                 double sentry_torino_scale,
                 double sentry_palermo_scale,
                 double sentry_diameter_km);

void item_apagar(ITEM **item);
void item_escrever_csv(const ITEM *item, FILE *arquivo);

#endif
